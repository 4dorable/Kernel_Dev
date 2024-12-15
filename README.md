# README - Projet Rootkit Linux

## Partie principale : Lancer l'Alpine avec le Rootkit et le Reverse Shell

1. **Configurer et lancer le système :**

   ```bash
    A remplir
   ```

2. **Compiler les modules :**

   ```bash
    A remplir
   ```

3. **Lancer l'Alpine avec les modules intégrés :**

   ```bash
   A remplir
   ```

### **2. Une fois dans l'Alpine**

1. **Remonter le proc et passer le système en mode lecture/écriture :**

   ```bash
   mount -t proc proc /proc
   mount -o remount,rw /
   ```

2. **Créer un utilisateur pour tester les modules :**

   ```bash
   adduser -h /home/testuser -s /bin/ash -u 1337 testuser
   ```

3. **Préparer la connexion au reverse shell :**

   ```bash
   nc -lvp 4444
   ```

4. **Charger les modules :**
   Rendez-vous dans le dossier contenant les modules, puis exécutez :

   ```bash
   cd lib/
   insmod privesc.ko
   insmod reverse.ko
   insmod hide_modules.ko
   ```

   Après avoir charger le module reverse vous devriez être connecter avec la machine attaquante

5. **Vérifier les modules :**

   - Si vous exécutez `lsmod`, les modules n'apparaîtront pas, car ils sont masqués.

6. **Configurer l'utilisateur :**

   ```bash
   echo 'testuser:x:1001:1001::/home/testuser:/bin/ash' >> /etc/passwd
   echo 'testuser:x:1001:' >> /etc/group
   mkdir -p /home/testuser
   chown -R 1001:1001 /home/testuser
   chmod -R 777 /home/testuser
   ```

7. **Se connecter avec l'utilisateur :**

   ```bash
   su - testuser
   ```

8. **Vérifier les privilèges :**
   En tant qu'utilisateur `testuser`, exécutez :

   ```bash
   id
   ```

   Vous constaterez que vous avez obtenu les privilèges root.

---

## Partie Backup : Lancer l'Alpine avec le Rootkit

### **1. Configurer et lancer le noyau Linux**

1. **Compiler le noyau Linux :**
   Assurez-vous que le noyau est correctement configuré et compilé avant de poursuivre.

### **Prérequis :**

1. **Installer GRUB avant de configurer le système sinon skip vers l'étape 2 :**

   ```bash
   sudo chmod +x install_grub.sh
   sudo ./install_grub.sh
   ```

2. **Configurer et lancer le système :**

   ```bash
   sudo chmod +x setup_system.sh
   sudo chmod +x rootkit_setup.sh
   sudo ./setup_system.sh
   sudo ./rootkit_setup.sh
   ```

3. **Permissions des fichiers :**
   Si des problèmes de permissions surviennent après l'exécution de `setup_system.sh`, utilisez :

   ```bash
   chmod -R 777 /script_nathan/modules_restrited
   ```

4. **Compiler les modules :**

   ```bash
   cd modules/
   make
   ```

   Pour nettoyer les modules :

   ```bash
   cd modules/
   make clean
   ```

5. **Lancer l'Alpine avec les modules intégrés :**

   ```bash
   ./rootkit_setup.sh
   ```

6. **Lancer l'Alpine Linux :**
   Une fois que l'image disque est prête, vous pouvez la lancer avec la commande suivante :

   ```bash
   qemu-system-x86_64 \
       -kernel linux-6.10.10/arch/x86/boot/bzImage \
       -append "root=/dev/sda1 console=ttyS0 init=/bin/sh" \
       -drive file=disk.img,format=raw \
       -nographic \
       -netdev user,id=net0,hostfwd=tcp::5555-:12345 -device e1000,netdev=net0
   ```

   Ou si vous voulez rebuild l'alpine entièrement :

   ```bash
   ./rootkit_setup.sh
   ```

### **Une fois dans l'Alpine**

1. **Remonter le proc et passer le système en mode lecture/écriture :**

   ```bash
   mount -t proc proc /proc
   mount -o remount,rw /
   ```

2. **Créer un utilisateur pour tester les modules :**

   ```bash
   adduser -h /home/testuser -s /bin/ash -u 1337 testuser
   ```

3. **Charger les modules :**
   Rendez-vous dans le dossier contenant les modules, puis exécutez :

   ```bash
   cd lib/
   insmod privesc.ko
   insmod hide_modules.ko
   ```

4. **Vérifier les modules :**

   - Si vous exécutez `lsmod`, les modules n'apparaîtront pas, car ils sont masqués.

5. **Configurer l'utilisateur :**

   ```bash
   echo 'testuser:x:1001:1001::/home/testuser:/bin/ash' >> /etc/passwd
   echo 'testuser:x:1001:' >> /etc/group
   mkdir -p /home/testuser
   chown -R 1001:1001 /home/testuser
   chmod -R 777 /home/testuser
   ```

6. **Se connecter avec l'utilisateur :**

   ```bash
   su - testuser
   ```

7. **Vérifier les privilèges :**
   En tant qu'utilisateur `testuser`, exécutez :

   ```bash
   id
   ```

   Vous constaterez que vous avez obtenu les privilèges root.

---

## Conclusion
