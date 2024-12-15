# Rootkit PrivEsc - Guide d'utilisation

## Description

Ce rootkit utilise un **kprobe** pour intercepter la syscall `getuid`. Si l'utilisateur possède un **UID spécifique (1337)**, les privilèges de cet utilisateur sont automatiquement élevés à **root**. Ce guide décrit comment configurer un environnement de test et utiliser la fonctionnalité de privilège escaladé.

---

## Étapes d'utilisation

### 1. Monter les systèmes de fichiers nécessaires

Avant de charger le module rootkit, il est important de s'assurer que le système de fichiers `/proc` est monté et que le système de fichiers principal est en mode lecture/écriture :

```bash
mount -t proc proc /proc
mount -o remount,rw /
```

---

### 2. Configurer le système pour l'utilisateur

1. Autoriser la commande `su` pour permettre au nouvel utilisateur de passer root :

   ```bash
   chmod u+s /bin/su
   ```

2. Créer un utilisateur avec un **UID 1337** et configurer son environnement :

   ```bash
   adduser -h /home/testuser -s /bin/ash -u 1337 testuser
   ```

3. Ajouter les informations de l'utilisateur manuellement (au cas où `adduser` ne fonctionnerait pas) :
   ```bash
   echo 'testuser:x:1337:1337::/home/testuser:/bin/ash' >> /etc/passwd
   echo 'testuser:x:1337:' >> /etc/group
   mkdir -p /home/testuser
   chown -R 1337:1337 /home/testuser
   chmod -R 777 /home/testuser
   ```

---

### 3. Charger le module rootkit

1. Placez-vous dans le répertoire où se trouve le module rootkit compilé (`privesc.ko`) :

   ```bash
   cd lib/
   ```

2. Chargez le module rootkit :
   ```bash
   insmod privesc.ko
   ```

---

### 4. Tester l'élévation des privilèges

1. Connectez-vous en tant que l'utilisateur `testuser` :

   ```bash
   su - testuser
   ```

2. Vérifiez les privilèges avec la commande `id` :

   ```bash
   id
   ```

   Si le rootkit fonctionne correctement, la sortie sera similaire à :

   ```
   uid=0(root) gid=0(root) groups=1337(testuser)
   ```

   Cela indique que l'utilisateur `testuser` a maintenant les privilèges de **root**.

---
