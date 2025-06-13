# Server & Client Multiplatform
#### Author: Bocaletto Luca


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
