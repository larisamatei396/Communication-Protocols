/* Matei Bianca-Larisa -> 322CB */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

/* Functia intoarce 1 daca trimite pachet de tip ACK si 0 in caz contrar */
int CheckMsg(Mini_kermit* recv_minikermit) {
  msg t;
  int ack = 0;
  unsigned short crc = crc16_ccitt(recv_minikermit, recv_minikermit -> LEN - 1);
  Mini_kermit* minikermit = Init_Minikermit();

  /* Trimit pachet de tip ACK sau NAK */
  if (crc == recv_minikermit -> CHECK) {
    minikermit -> TYPE = 'Y';
    ack = 1;
  }
  else {
    minikermit -> TYPE = 'N';
  }

  /* Copiez datele pe care trebuie sa le trimit in pachetul minikermit */
  memcpy(&minikermit -> SOH, &recv_minikermit -> SOH, 1);
  memcpy(&minikermit -> SEQ, &recv_minikermit -> SEQ, 1);
  memcpy(&minikermit -> CHECK, &recv_minikermit -> CHECK, 2);
  memcpy(&minikermit -> MARK, &recv_minikermit -> MARK, 1);

  if (minikermit -> TYPE == 'Y' && recv_minikermit -> TYPE == 'S') {
    memcpy(&minikermit -> LEN, &recv_minikermit -> LEN, 1);
    memcpy(&minikermit -> DATA, &recv_minikermit -> DATA, minikermit -> LEN - 5);
  }
  else
    minikermit -> LEN = 5;

  /* Incrementez SEQ in cazul in care trimit pachet de tip ACK */
  if (minikermit -> TYPE == 'Y')
    minikermit -> SEQ++;
    minikermit -> SEQ = minikermit -> SEQ % 64;

  /* Creez mesaj si il trimit (ACK sau NAK) */
  t = CreateMsg(minikermit);
  send_message(&t);
  return ack;
}

int main(int argc, char** argv) {

  init(HOST, PORT);

  msg *y = NULL;
  Mini_kermit *recv_minikermit;
  FILE* file;
  char* name = (char*) malloc(100 * sizeof(char));
  int ack = 0;

  /* Cat timp nu a ajuns la finalul transmisiei */
  while (recv_minikermit -> TYPE != 'B' || ack == 0) {

    y = receive_message_timeout(15000);
    recv_minikermit = Convert(*y);
    printf("[%d] Got package: ", recv_minikermit -> SEQ);

    switch (recv_minikermit -> TYPE) {
      case 'S':
        printf("Send Init\n");
        break;
      case 'F':
        printf("File Header\n");
        break;
      case 'D':
        printf("Data\n");
        break;
      case 'Z':
        printf("EOF\n");
        break;
      default:
        printf("EOT\n");
        break;
    }
    ack = CheckMsg(recv_minikermit);

    /* Daca pachetul este de tip File Header, deschid un nou fisier pentru
        scriere */
    if (recv_minikermit -> TYPE == 'F' && ack == 1) {
      memcpy(name, "recv_", 5);
      memcpy(name + 5, recv_minikermit -> DATA, recv_minikermit -> LEN - 5);
      file = fopen(name, "wb");
    }

    /* Daca pachetul este de tip Date, scriu datele in fisierul curent */
    if (recv_minikermit -> TYPE == 'D' && ack == 1) {
      fwrite(recv_minikermit -> DATA, sizeof (char), recv_minikermit -> LEN - 5,
              file);
    }

    /* Daca pachetul este de tipul EOF, inchid fisierul */
    if (recv_minikermit -> TYPE == 'Z' && ack == 1) {
      fclose(file);
    }
  }
  free(name);
	return 0;
}
