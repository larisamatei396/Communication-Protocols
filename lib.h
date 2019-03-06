/* Matei Bianca-Larisa -> 322CB */
#ifndef LIB
#define LIB

typedef struct __attribute__((__packed__)) {
    int len;
    char payload[1400];
} msg;

/* Structura pentru pachet mini-kermit */
typedef struct __attribute__((__packed__)) {
  char SOH;
  unsigned char LEN;
  unsigned char SEQ;
  char TYPE;
  char DATA[250];
  unsigned short CHECK;
  char MARK;
} Mini_kermit;

/* Structura pentru pachet de tip S */
typedef struct __attribute__((__packed__)) {
  unsigned char MAXL;
  unsigned char TIME;
  char NPAD;
  char PADC;
  char EOL;
  char QCTL;
  char QBIN;
  char CHKT;
  char REPT;
  char CAPA;
  char R;
} SendInit_pkg;

/* Antet functii */
void init(char* remote, int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);
msg* receive_message_timeout(int timeout); //timeout in milliseconds
unsigned short crc16_ccitt(const void *buf, int len);

/* Functie care genereaza si initializeaza un pachet mini-kermit */
Mini_kermit* Init_Minikermit() {
  Mini_kermit* minikermit = (Mini_kermit*) malloc (sizeof(Mini_kermit));

  /* Alocare nereusita */
  if (!minikermit) {
    fprintf(stderr, "Alocare nereusita\n");
    return NULL;
  }
  minikermit -> SOH = 0x01;
  minikermit -> SEQ = 0x00;

  return minikermit;
}

/* Functie care creeaza un mesaj */
msg CreateMsg(Mini_kermit* minikermit) {
  msg t;
  memcpy(t.payload, &minikermit -> SOH, 1);
  memcpy(t.payload + 1, &minikermit -> LEN, 1);
  memcpy(t.payload + 2, &minikermit -> SEQ, 1);
  memcpy(t.payload + 3, &minikermit -> TYPE, 1);
  memcpy(t.payload + 4, minikermit -> DATA, minikermit -> LEN - 5);
  memcpy(t.payload + minikermit -> LEN - 1, &minikermit -> CHECK, 2);
  memcpy(t.payload + minikermit -> LEN + 1, &minikermit -> MARK, 1);
  t.len = strlen(t.payload);
  return t;
}

/* Functie care converteste din msg in pachet mini-kermit */
Mini_kermit* Convert(msg t) {
  Mini_kermit* minikermit = Init_Minikermit();
  memcpy(&minikermit -> SOH, t.payload, 1);
  memcpy(&minikermit -> LEN, t.payload + 1, 1);
  memcpy(&minikermit -> SEQ, t.payload + 2, 1);
  memcpy(&minikermit -> TYPE, t.payload + 3, 1);
  memcpy(minikermit -> DATA, t.payload + 4, minikermit -> LEN - 5);
  memcpy(&minikermit -> CHECK, t.payload + minikermit -> LEN - 1, 2);
  memcpy(&minikermit -> MARK, t.payload + minikermit -> LEN + 1, 1);
  return minikermit;
}

#endif
