## Description

Ce module du noyau Linux est conçu pour fournir discrétion et protection pour des modules spécifiques. Il cache les modules spécifiés de la liste des modules du noyau et les protège contre leur déchargement en augmentant leur compteur de références. De plus, le module se cache lui-même de la liste des modules du noyau et du sysfs, assurant une discrétion maximale.

---

## Fonctionnalités

- **Cacher des modules du noyau spécifiques** : Supprime des modules spécifiés (par exemple, `reverse`, `privesc`) de la liste des modules du noyau.
- **Protéger des modules spécifiques** : Augmente le compteur de références des modules spécifiés pour empêcher leur déchargement.
- **Se cacher lui-même** : Garantit que le module `hide_modules` est invisible dans la liste des modules du noyau et dans le sysfs.
- **Opérations réversibles** : Permet de restaurer les modules cachés et de rendre le module `hide_modules` visible à nouveau lors du nettoyage.

---

## Fonctionnement

1. **Cacher des Modules** :
   - La fonction `hide_module` supprime le module cible de la liste chaînée des modules du noyau.
   - La fonction `protect_module` appelle `try_module_get()` pour augmenter le compteur de références du module.
2. **Se Cacher** :
   - La fonction `hide_self` supprime le module `hide_modules` de la liste des modules du noyau et du sysfs.
3. **Nettoyage** :
   - La fonction `hide_modules_exit` restaure les modules cachés dans la liste des modules du noyau et diminue leur compteur de références.

---

## Installation et Utilisation

### Prérequis

- Assurez-vous d'avoir un noyau qui permet le chargement de modules non signés.
- Ayez un accès administratif/root au système.

### Compilation

1. Copiez le code source dans un fichier nommé `hide_modules.c`.
2. Compilez le module :
   ```bash
   make
   ```

### Chargement du Module

1. Insérez le module compilé dans le noyau :
   ```bash
   insmod hide_modules.ko
   ```
   Cela :
   - Cache les modules `reverse` et `privesc` s'ils sont chargés.
   - Protège ces modules contre leur déchargement.
   - Cache le module `hide_modules` lui-même.

### Vérification des Modules Cachés

- Pour vérifier si les modules spécifiés sont cachés, listez les modules chargés :
  ```bash
  cat /proc/modules
  ```
  Les modules cachés ne devraient pas apparaître dans la sortie.

### Déchargement du Module

1. Restaurez les modules cachés et rendez le module `hide_modules` visible :
   ```bash
   rmmod hide_modules.ko
   ```

---

## Exemple d'Utilisation

1. Chargez les modules `reverse` et `privesc` :
   ```bash
   insmod reverse.ko
   insmod privesc.ko
   ```
2. Chargez le module `hide_modules` :
   ```bash
   insmod hide_modules.ko
   ```
3. Vérifiez que `reverse`, `privesc` et `hide_modules` sont cachés.
4. Pour nettoyer, déchargez le module `hide_modules` :
   ```bash
   rmmod hide_modules.ko
   ```

---
