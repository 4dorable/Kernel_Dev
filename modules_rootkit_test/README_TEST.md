# README - Projet Rootkit Linux

## Introduction

Ce document résume les différentes parties du projet de rootkit Linux, incluant les modules implémentés et les essais effectués pour atteindre les objectifs fixés. Chaque section expliquera le fonctionnement des modules, les technologies utilisées, et les résultats obtenus.

---

## Modules Implémentés

### **1. `kill.c` - Hook de syscall pour l'élévation des privilèges**

- **Description** :
  Ce module utilise un hook sur le syscall `kill` (à travers les Kprobes) pour attribuer des privilèges root à un processus lorsque celui-ci envoie un signal spécifique (dans ce cas, le signal 62).

- **Objectif** :
  Comprendre comment utiliser les Kprobes pour détourner un appel système et exploiter cette technique pour fournir des privilèges élevés.

- **Statut** : Fonctionnel et testé avec succès.

- **Technologies Utilisées** :

  - Kprobes pour intercepter le syscall.
  - Gestion des credentials pour attribuer les privilèges root au processus appelant.

- **Limitation** : Bien que ce module soit fonctionnel, il est interdit dans le cadre du projet d'utiliser l'appel système `kill` pour l'élévation des privilèges.

---

### **2. `open.c` - Tentative d'élévation des privilèges via le syscall openat**

- **Description** :
  Ce module tente d'utiliser un hook sur le syscall `openat` pour éléver les privilèges lorsqu'un utilisateur ouvre un fichier spécifique (à définir dans le code).

- **Objectif** :
  Explorer une autre méthode que `kill` pour obtenir des privilèges root en interceptant un syscall différent et en détectant une condition précise.

- **Statut** :

  - Implémenté et compilé sans erreur.
  - Aucune erreur d'exécution.
  - Cependant, les privilèges root n'étaient pas accordés à l'utilisateur malgré l'exécution du fichier spécifique. Le problème n'a pas été résolu par manque de temps.

- **Technologies Utilisées** :

  - Kprobes pour intercepter `openat`.
  - Gestion des credentials pour attribuer des privilèges root.

- **Limitation** : Les privilèges root n'ont pas été accordés comme prévu.

---

### **3. `hide_user.c` - Tentative de dissimulation d'un utilisateur**

- **Description** :
  Ce module a pour but de cacher un utilisateur spécifique dans le système. L'objectif était d'utiliser les hooks sur les syscalls pour manipuler les données retournées par le noyau, notamment lors de la lecture des fichiers `passwd` ou `shadow`.

- **Objectif** :
  Fournir une méthode pour masquer totalement l'existence d'un utilisateur (ex. : UID 1337) sur le système.

- **Statut** :

  - Code implémenté mais non fonctionnel.
  - Par manque de temps, le module n'a pas atteint son objectif de cacher efficacement l'utilisateur.

- **Technologies Utilisées** :
  - Hooks sur les syscalls.
  - Manipulation des structures de données utilisateur.

---

### **4. `persistence.c` - Module de persistance avec écriture dans `rc.local`**

- **Description** :
  Ce module écrit dans le fichier `/etc/rc.local` pour insérer les modules de rootkit (à savoir `hide_modules`, `reverse` et `privesc`) au démarrage du système. En cas d'absence de `rc.local`, il crée le fichier et y ajoute les commandes nécessaires. De plus, il gère également un service OpenRC pour garantir que le fichier `rc.local` est exécuté.

- **Objectif** :
  Assurer la persistance des rootkits au redémarrage du système.

- **Statut** :

  - Le module crée correctement le fichier `/etc/rc.local` et y ajoute les commandes nécessaires.
  - En revanche, le contenu de `rc.local` est effacé au redémarrage du système. La raison n'a pas pu être identifiée par manque de temps.

- **Technologies Utilisées** :

  - API du noyau pour créer et modifier des fichiers.
  - Intégration avec OpenRC via la création d'un script dans `/etc/init.d/`.

- **Limitation** :
  - La persistance n'est pas effective car le fichier `rc.local` se vide au redémarrage.
  - Par manque de temps, il n'a pas été possible de diagnostiquer et corriger ce problème.

---

## Conclusion

Ce projet a permis d'explorer diverses techniques pour créer un rootkit fonctionnel et persistant. Bien que certaines fonctionnalités soient pleinement implémentées (comme le hook de syscall avec `kill`), d'autres sont restées incomplètes par manque de temps ou de solutions adaptées. Le module `persistence.c` représente un état proche d'une solution complète pour la persistance, mais le problème lié à la vidange du fichier `rc.local` au redémarrage reste un obstacle à surmonter.
