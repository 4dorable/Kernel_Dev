Le but est de faire un rootkit (r0) dans un systeme Linux. 
Educational purpose only. 

Le but est de comprendre le fonctionnement et le developpement de Modules / Syscalls sur Linux ainsi que le hooking de fonctions.
Mon rootkit doit etre discret, avoir un Command&Control et d'autres fonctionnalites. 

Je ne dois pas use kill().

Un script Bash permet de lancer une machine Alpine LFS (Linux From Scratch) afin de tester mon RootKit. 
Il y a 2 users, un root et un user avec des logins differents.

Voici une liste de chose que j'aimerai faire : 
- Un Keylogger
- Cacher sa presence
- Backdoor
- Persistence
- Escalade de Privileges.
