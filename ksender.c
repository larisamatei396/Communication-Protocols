/* Matei Bianca-Larisa -> 322CB */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

/* Functie care genereaza si initializeaza un pachet de tip S */
SendInit_pkg* Init_SendInitpkg() {
  SendInit_pkg* sendinit = (SendInit_pkg*) malloc (sizeof(SendInit_pkg));

  /* Alocare nereusita */
  if (!sendinit) {
    fprintf(stderr, "Alocare nereusita\n");
    return NULL;
  }
  sendinit -> NPAD = 0x00;
  sendinit -> PADC = 0x00;
  sendinit -> EOL = 0x0D;
  sendinit -> QCTL = 0x00;
  sendinit -> QBIN = 0x00;
  sendinit -> CHKT = 0x00;
  sendinit -> REPT = 0x00;
  sendinit -> CAPA = 0x00;
  sendinit -> R = 0x00;

  return sendinit;
}

/* Functie care trimite un mesaj si asteapta un raspuns (ACK sau NAK)
    In caz de timeout sau NAK, retrimite pachetul. Un pachet este trimis de
    maxim 3 ori */
int Send (msg t) {
  int nr = 0;
  Mini_kermit* recv_minikermit;
  msg *y = NULL;
  while (y == NULL || recv_minikermit -> TYPE == 'N') {
    send_message(&t);
    y = receive_message_timeout(5000);

    if (y != NULL) {
      recv_minikermit = Convert(*y);
      printf("[%d] Got reply: ", recv_minikermit -> SEQ);
      switch (recv_minikermit -> TYPE) {
        case 'Y':
          printf("ACK\n");
          break;
        default:
          printf("NAK\n");
          break;
      }

    if (recv_minikermit -> TYPE == 'Y')
      break;
    }
    nr++;
    if (nr == 3) {
      printf("===========================\n");
      if (recv_minikermit -> TYPE == 'N' && y == NULL)
        printf("Intrerupere cauzata corupere si timeout!\n");
      else if (recv_minikermit -> TYPE == 'N' && y != NULL)
        printf("Intrerupere cauzata de corupere!\n");
      else
        printf("Intrerupere cauzata de timeout!\n");
      return -1;
    }
  }
  return 0;
}

int main(int argc, char** argv) {
    msg t;
    init(HOST, PORT);
    int crt_file = 1, offset = 250;
    FILE* file;

    /* Generez pachet de tip Send Init si il trimit */
    Mini_kermit* minikermit = Init_Minikermit();
    SendInit_pkg* sendinit = Init_SendInitpkg();
    minikermit -> LEN = sizeof(SendInit_pkg) + 5;
    memcpy(minikermit -> DATA, sendinit, sizeof(SendInit_pkg));
    minikermit -> TYPE = 'S';
    minikermit -> CHECK = crc16_ccitt(minikermit, minikermit -> LEN - 1);

    t = CreateMsg(minikermit);
    if (Send(t) == -1)
      return 0;
    minikermit -> SEQ++;
    minikermit -> SEQ = minikermit -> SEQ % 64;

    /* Cat timp am fisiere pe care trebuie sa le trimit */
    while (crt_file < argc) {
      /* Deschid fisierele */
      file = fopen(argv[crt_file], "rb");
      /* Trimit pachet de tip File Header */
      minikermit -> LEN = strlen(argv[crt_file]) + 5;
      minikermit -> SEQ++;
      minikermit -> SEQ = minikermit -> SEQ % 64;
      memcpy(minikermit -> DATA, argv[crt_file], strlen(argv[crt_file]));
      minikermit -> TYPE = 'F';
      minikermit -> CHECK = crc16_ccitt(minikermit, minikermit -> LEN - 1);

      t = CreateMsg(minikermit);
      if (Send(t) == -1)
        return 0;
      minikermit -> SEQ++;
      minikermit -> SEQ = minikermit -> SEQ % 64;

      /* Cat timp pot citi din fisier, generez pachete de tip Date si le
        trimit */
      offset = 250;
      while (offset == 250) {
        offset = fread(minikermit -> DATA, sizeof(char), offset, file);
        minikermit -> LEN = offset + 5;
        minikermit -> SEQ++;
        minikermit -> SEQ = minikermit -> SEQ % 64;
        minikermit -> TYPE = 'D';
        minikermit -> CHECK = crc16_ccitt(minikermit, minikermit -> LEN - 1);

        t = CreateMsg(minikermit);
        if (Send(t) == -1)
          return 0;
        minikermit -> SEQ++;
        minikermit -> SEQ = minikermit -> SEQ % 64;
      }

      /* Trimit pachet de tip EOF */
      minikermit -> LEN = 5;
      minikermit -> SEQ++;
      minikermit -> SEQ = minikermit -> SEQ % 64;
      minikermit -> TYPE = 'Z';
      minikermit -> CHECK = crc16_ccitt(minikermit, 4);

      t = CreateMsg(minikermit);
      if (Send(t) == -1)
        return 0;
      minikermit -> SEQ++;
      minikermit -> SEQ = minikermit -> SEQ % 64;

      fclose(file);
      crt_file++;
    }

    /* Trimit pachet de tip EOT */
    minikermit -> LEN = 5;
    minikermit -> SEQ++;
    minikermit -> SEQ = minikermit -> SEQ % 64;
    minikermit -> TYPE = 'B';
    minikermit -> CHECK = crc16_ccitt(minikermit, 4);

    t = CreateMsg(minikermit);
    if (Send(t) == -1)
      return 0;
    minikermit -> SEQ++;
    minikermit -> SEQ = minikermit -> SEQ % 64;

    return 0;
}
