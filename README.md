# SISOP-1-2026-IT-108  
## The Wired — Present Day, Present Time

  Program ini merupakan implementasi sistem komunikasi **client-server menggunakan socket TCP pada bahasa C**.  
Sistem ini mensimulasikan jaringan digital bernama **The Wired**, dimana berbagai entitas dapat terhubung, berkomunikasi secara real-time, dan seluruh aktivitasnya tercatat dalam sistem log.

---

### Latar Belakang Masalah

Dalam dunia digital modern, komunikasi antar entitas dalam jaringan membutuhkan sebuah sistem yang mampu menghubungkan banyak pengguna secara bersamaan dengan stabil dan efisien.

Konsep **The Wired** menggambarkan sebuah jaringan kolektif dimana identitas digital dapat saling terhubung dan berinteraksi secara real-time.

Berdasarkan deskripsi soal, sistem ini harus mampu:

1. Menghubungkan banyak client ke dalam satu server pusat.
2. Menyediakan komunikasi real-time antar client.
3. Mengelola identitas unik setiap pengguna.
4. Menyediakan kontrol administratif melalui entitas khusus bernama **The Knights**.
5. Mencatat seluruh aktivitas jaringan dalam file log untuk keperluan analisis.

Untuk memenuhi kebutuhan tersebut, sistem ini diimplementasikan menggunakan **socket programming pada bahasa C dengan arsitektur client-server**.


### Arsitektur Sistem

Program ini menggunakan arsitektur **Client-Server**.

Server (`wired.c`) bertugas untuk:

- menerima koneksi client
- mengelola komunikasi antar client
- melakukan broadcast pesan
- mencatat aktivitas sistem
- menyediakan console admin

Client (`navi.c`) bertugas untuk:

- terhubung ke server
- mengirim pesan
- menerima pesan dari client lain

---

### Struktur File Program

| File | Fungsi |
|-----|------|
| `wired.c` | Program server utama |
| `navi.c` | Program client |
| `protocol.h` | Konfigurasi komunikasi |
| `protocol.c` | Include protocol |
| `history.log` | File log aktivitas |

---

### Konfigurasi Protocol

Konfigurasi komunikasi client-server disimpan dalam file `protocol.h`.

```c
#define PORT 8080
#define IP "127.0.0.1"
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 30

#define ADMIN_NAME "The Knights"
#define ADMIN_PASS "protocol7"
```
## Fitur Program
**1 Multi Client Connection**
Server mampu menangani banyak client sekaligus menggunakan select().

**2 Broadcast Message**
Pesan yang dikirim oleh satu client akan diteruskan ke client lain.

```c [alice]: hello lain
[lain]: hello alice
```

**3 Username Unik**
Server akan menolak username yang sudah digunakan.

```c [System] The identity is already synchronized in The Wired.
```
**4. Command Exit**
Client dapat keluar dengan command:
```c
/exit
```
Output:
```c
[System] Disconnecting from The Wired...
```
**5 Admin Console (The Knights)**

Login sebagai admin:

```c Enter your name: The Knights
Enter Password: protocol7
```

Menu admin:
```c
=== THE KNIGHTS CONSOLE ===
1. Check Active Entities (Users)
2. Check Server Uptime
3. Execute Emergency Shutdown
4. Disconnect
```
**Sistem Logging**
Semua aktivitas server dicatat pada file history.log.

Format log:
```c
[YYYY-MM-DD HH:MM:SS] [Role] [Event]
```
Contoh:
```c
[2026-04-26 19:06:40] [System] [SERVER ONLINE]
[2026-04-26 19:06:46] [System] [User 'alice' connected]
[2026-04-26 19:06:50] [System] [User 'lain' connected]
[2026-04-26 19:06:56] [User] [[alice]: hello lain]
```

## Penjelasan Kode

**1). Navi.c**
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "protocol.h"
```
Kode ini digunakan untuk mengimpor berbagai library yang dibutuhkan oleh program. Library seperti stdio.h dan stdlib.h digunakan untuk operasi dasar C, sedangkan arpa/inet.h dan sys/socket.h digunakan untuk kebutuhan socket programming. Library pthread.h digunakan agar client dapat menjalankan thread untuk menerima pesan secara bersamaan dengan pengiriman pesan.

```c
void *receive_message(void *arg) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(sock, buffer, BUFFER_SIZE);

        if (valread <= 0) {
            break;
        }

        printf("%s", buffer);
        fflush(stdout);
    }

    return NULL;
}
```
Fungsi ini digunakan untuk menerima pesan dari server secara terus menerus. Program membaca data dari socket kemudian menampilkannya ke layar menggunakan printf. Fungsi ini dijalankan dalam thread terpisah sehingga client tetap dapat menerima pesan walaupun sedang menulis pesan baru.

```c
int main() {
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char name[50];
    pthread_t recv_thread;

    sock = socket(AF_INET, SOCK_STREAM, 0);
```
Pada bagian ini program membuat socket client menggunakan protokol TCP (SOCK_STREAM). Struktur sockaddr_in digunakan untuk menyimpan informasi alamat server seperti IP dan port.

```c
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
```
Kode ini digunakan untuk mengatur alamat server yang akan dihubungi oleh client. Fungsi connect() kemudian digunakan untuk melakukan koneksi ke server yang sedang berjalan.

```c
    printf("Enter your name: ");
    fgets(name, 50, stdin);
    send(sock, name, strlen(name), 0);
```
Bagian ini digunakan untuk meminta input nama dari pengguna. Nama tersebut kemudian dikirim ke server sebagai identitas client yang terhubung ke jaringan The Wired.

```c
    pthread_create(&recv_thread, NULL, receive_message, NULL);
```
Kode ini membuat thread baru yang menjalankan fungsi receive_message. Dengan adanya thread ini client dapat menerima pesan dari server secara paralel tanpa mengganggu proses pengiriman pesan.

```c
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        send(sock, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "/exit", 5) == 0) {
            break;
        }
    }
```
 Loop ini digunakan untuk membaca pesan yang dimasukkan oleh pengguna lalu mengirimkannya ke server. Jika pengguna mengetik perintah /exit, maka client akan keluar dari program dan koneksi akan ditutup.


**2). protocol.c**
```c
#include "protocol.h"
```
File ini hanya berfungsi untuk menghubungkan implementasi program dengan konfigurasi yang telah didefinisikan pada protocol.h. Pada program ini file tersebut tidak memiliki fungsi tambahan, namun tetap disediakan untuk menjaga struktur program agar tetap modular.

**3). protocol.h**
```c
#ifndef PROTOCOL_H
#define PROTOCOL_H
```
Bagian ini merupakan header guard yang berfungsi untuk mencegah file header dimuat lebih dari satu kali saat proses kompilasi.

```c
#define PORT 8080
#define IP "127.0.0.1"
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 30
```
Konstanta ini digunakan sebagai konfigurasi dasar komunikasi jaringan. PORT menentukan port server, IP menentukan alamat server, BUFFER_SIZE menentukan ukuran buffer pesan, dan MAX_CLIENTS menentukan jumlah maksimal client yang dapat terhubung.

```c
#define ADMIN_NAME "The Knights"
#define ADMIN_PASS "protocol7"
```
Bagian ini mendefinisikan username dan password admin yang digunakan untuk mengakses console administrasi server.

**4).Wired**
**Inisialisasi Variable Global**
```c
int clients[MAX_CLIENTS];
char names[MAX_CLIENTS][50];
int client_count = 0;
time_t start_time;
```
Variabel ini digunakan untuk menyimpan daftar client yang terhubung ke server. Array clients menyimpan socket client, sedangkan names menyimpan nama pengguna yang terhubung. Variabel client_count digunakan untuk menghitung jumlah client aktif, dan start_time digunakan untuk mencatat waktu ketika server mulai dijalankan.

**Fungsi write.log**
```c
void write_log(const char *type, const char *message) {
    FILE *fp = fopen("history.log", "a");
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] %s %s\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec,
        type, message);

    fclose(fp);
}
```
Fungsi ini digunakan untuk mencatat aktivitas server ke dalam file history.log. Program mengambil waktu sistem saat ini, kemudian menuliskan informasi kejadian beserta timestamp ke dalam file log.

**Fungsi name_exist**
```c
int name_exists(char *name) {
    for (int i = 0; i < client_count; i++) {
        if (strcmp(names[i], name) == 0) return 1;
    }
    return 0;
}
```
Fungsi ini digunakan untuk mengecek apakah nama pengguna yang ingin masuk ke server sudah digunakan oleh client lain. Jika nama sudah ada maka server akan menolak koneksi tersebut.

**Fungsi broadcast**
```c
void broadcast(int sender, char *msg) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender) {
            send(clients[i], msg, strlen(msg), 0);
        }
    }
}
```
Fungsi ini digunakan untuk mengirim pesan dari satu client ke semua client lain yang sedang terhubung ke server. Server akan melakukan iterasi pada seluruh socket client dan mengirim pesan kecuali kepada pengirim.

**Fungsi remove_client**
```c
void remove_client(int index) {
    char logmsg[100];
    sprintf(logmsg, "[User '%s' disconnected]", names[index]);
    write_log("[System]", logmsg);

    close(clients[index]);

    for (int i = index; i < client_count - 1; i++) {
        clients[i] = clients[i + 1];
        strcpy(names[i], names[i + 1]);
    }

    client_count--;
}
```
Fungsi ini digunakan ketika client keluar dari server. Server akan menutup socket client, menghapus data client dari array, kemudian mengurangi jumlah client yang sedang aktif.

**Fungsi handle_admin**
```c
void handle_admin(int sock)
```
Fungsi ini digunakan untuk menangani perintah dari admin The Knights. Admin memiliki beberapa command seperti melihat daftar user aktif, mengecek uptime server, serta melakukan emergency shutdown.

**Fungsi main server**
```c
server_fd = socket(AF_INET, SOCK_STREAM, 0);
bind(server_fd, (struct sockaddr *)&address, sizeof(address));
listen(server_fd, 3);
```
Pada bagian ini server membuat socket TCP, melakukan binding ke port yang telah ditentukan, lalu mulai mendengarkan koneksi client yang masuk.

```c
select(max_sd + 1, &readfds, NULL, NULL, NULL);
```
Fungsi select() digunakan agar server dapat memonitor banyak socket sekaligus. Dengan metode ini server dapat menangani banyak client tanpa perlu membuat thread untuk setiap koneksi.

## Output Program

Berikut merupakan dokumentasi hasil pengujian program **The Wired** yang telah dijalankan melalui terminal WSL. Pengujian dilakukan untuk memastikan fitur client-server, autentikasi admin, komunikasi antar user, pengecekan user aktif, uptime server, emergency shutdown, serta pencatatan log berjalan sesuai kebutuhan.

### 1. Tampilan Awal Program

![Output WSL pertama](output%20wsl%20ke%20tiga.png)

Program dijalankan melalui terminal WSL dengan masuk ke direktori project, kemudian menjalankan executable `./navi`.

![Output WSL kedua](output%20wsl%20ke%20dua.png)

Pada tahap ini, client berhasil masuk ke sistem menggunakan identitas user.

---

### 2. Login sebagai User

![Output User Alice](output%20lain.png)

User `alice` berhasil masuk ke dalam sistem The Wired dan menerima pesan dari user lain.

![Output User Lain](output%20coba%20nama%20yg%20sama.png)

Program juga melakukan validasi identitas agar nama user yang sama tidak dapat digunakan secara bersamaan.

---

### 3. Login sebagai Admin The Knights

![Output The Knights](output%20the%20knight.png)

Admin berhasil login menggunakan nama **The Knights** dan password yang sesuai. Setelah autentikasi berhasil, sistem menampilkan menu khusus admin.

---

### 4. Fitur Admin: Melihat User Aktif

![Output TK 1](output%20TK%201.png)

Admin dapat melihat daftar user yang sedang aktif di dalam sistem. Pada contoh ini, user `alice` dan `lain` sedang terhubung.

---

### 5. Fitur Admin: Melihat Server Uptime

![Output TK 2](output%20TK%202.png)

Admin dapat mengecek lama waktu server berjalan menggunakan fitur **Check Server Uptime**.

---

### 6. Fitur Admin: Emergency Shutdown

![Output TK 3](output%20TK3.png)

Admin dapat menjalankan perintah **Emergency Shutdown** untuk menghentikan server.

![Output TK 4](output%20TK%204.png)

Setelah perintah shutdown dijalankan, sistem menampilkan pesan bahwa emergency shutdown telah diinisiasi.

---

### 7. History Log

![Output History](output%20history.png)

Seluruh aktivitas sistem tercatat pada file `history.log`, seperti koneksi user, pesan antar client, perintah admin, uptime server, dan emergency shutdown.

---

## Kesimpulan Output

Berdasarkan hasil pengujian, program berhasil menjalankan fitur utama yang diminta, yaitu:

1. Client dapat terhubung ke server.
2. User dapat mengirim dan menerima pesan secara real-time.
3. Sistem dapat mencegah penggunaan nama user yang sama.
4. Admin **The Knights** dapat login menggunakan password khusus.
5. Admin dapat melihat user aktif.
6. Admin dapat mengecek uptime server.
7. Admin dapat menjalankan emergency shutdown.
8. Seluruh aktivitas tercatat pada file `history.log`.

## Kendala yang Dihadapi

Selama pengerjaan **Soal 1 – The Wired**, terdapat beberapa kendala yang dialami, di antaranya:

### 1. Sinkronisasi Komunikasi Antar Client
Pada awal implementasi, pesan yang dikirim oleh satu client tidak langsung diterima oleh client lain. Hal ini disebabkan oleh mekanisme broadcast pada server yang belum berjalan dengan benar. Kendala ini diatasi dengan memperbaiki alur pengiriman pesan agar dapat diteruskan ke seluruh client yang terhubung.

### 2. Validasi Username Unik
Kendala lain adalah memastikan setiap user memiliki username yang unik. Pada awalnya, sistem masih memperbolehkan penggunaan nama yang sama oleh beberapa client. Hal ini kemudian diperbaiki dengan menambahkan pengecekan di sisi server untuk menolak username yang sudah digunakan.