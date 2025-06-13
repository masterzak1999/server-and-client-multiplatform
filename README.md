# Server & Client
#### Author: Bocaletto Luca

# 🔧 Remote-Cmd v3

**Server & Client in C con autenticazione, syslog e graceful shutdown**

---

## ⚙️ Compilazione

	compila il server:
		gcc -Wall -O2 -pthread -o server server.c

	compila il client:
		gcc -Wall -O2 -o client client.c

---

## 🚀 Avvio Server

	# Avvio con autenticazione e aggiornamento del sistema
	sudo ./server -k mysecret -u

	# Modalità dry-run (non esegue comandi reali)
	./server -k mysecret -d

---

## 🧪 Connessione Client

	# Dry-run (visualizza i comandi senza inviarli)
	./client -h 127.0.0.1 -p 12345 -k mysecret -d

	# Connessione reale
	./client -h 127.0.0.1 -p 12345 -k mysecret

	Esempio interazione:

		ls -l
		uname -a
		exit

---

## 🔐 Autenticazione

	Il server richiede un token di autenticazione passato via `-k mysecret`.

	Il client lo invia alla connessione. In caso di token errato, riceverà `AUTH_FAIL`.

---

## ⚙️ Opzioni Server

	-p, --port PORT     porta di ascolto (default 12345)
	-k, --key TOKEN     token di autenticazione (obbligatorio)
	-u, --update        esegue apt-get update && upgrade -y
	-d, --dry-run       stampa i comandi senza eseguirli
	-s, --syslog        abilita log su syslog
	-V, --version       mostra la versione
	-h, --help          mostra l'aiuto

---

## ⚙️ Opzioni Client

	-h, --host HOST     IP o hostname del server
	-p, --port PORT     porta TCP del server
	-k, --key TOKEN     token di autenticazione
	-d, --dry-run       stampa i comandi senza inviarli
	-V, --version       mostra la versione
	-?, --help          mostra l'aiuto

---

## 📝 Note

- Il server supporta IPv4/IPv6, multi-client con thread, chiusura controllata via SIGINT/SIGTERM.
- Tutti i comandi ricevuti vengono eseguiti via `popen()` e il loro output è reinviato al client.
- Il client si comporta come una shell remota semplificata.
- La connessione termina scrivendo `exit`.

---

Bocaletto Luca – Software libero scritto in C, nessuna dipendenza esterna, compatibile con Debian/Linux.
