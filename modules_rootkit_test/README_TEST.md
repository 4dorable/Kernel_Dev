## Description du Projet

Ce projet explore la création et l’utilisation de rootkits pour Linux en manipulant des appels système et des modules kernel. Trois principaux codes ont été développés pour expérimenter et mettre en œuvre des fonctionnalités variées :

1. **Hook sur `kill` pour élévation de privilèges** : Fonctionnalité implémentée et testée avec succès.
2. **Hook sur `open` pour élévation de privilèges** : Tentative réalisée sans succès, bien que le code ait été fonctionnel.
3. **Cacher un utilisateur** : Fonctionnalité non finalisée par manque de temps.

---

## Détails des Modules

### 1. **Hook sur `kill` pour obtenir les privilèges root**

Ce module utilise un hook sur l'appel système `kill` pour accorder les privilèges root à l'utilisateur. Lorsqu'un signal spécifique (par exemple, `SIGUSR1`) est envoyé, le module modifie les credentials du processus appelant pour les définir en tant que root.

#### Fonctionnalités principales :

- **Détournement de l'appel `kill`** : Grâce à un kprobe, l'appel est intercepté avant son exécution.
- **Élévation des privilèges** : Lors de la réception du signal défini, les credentials sont modifiés pour accorder les privilèges root.

#### Résultats :

- Fonctionnalité testée avec succès. L'utilisateur a pu obtenir les privilèges root comme prévu.

---

### 2. **Hook sur `open` pour obtenir les privilèges root**

Une tentative a été réalisée pour intercepter l'appel système `open`. L'objectif était d'accorder les privilèges root lorsque l'utilisateur tentait d'accéder à un fichier spécifique (nommé `trigger_file`).

#### Fonctionnalités principales :

- **Détournement de l'appel `open`** : Le module intercepte l'ouverture de fichiers pour vérifier si le fichier cible est accédé.
- **Élévation des privilèges** : Si le fichier déclencheur est détecté, les credentials de l'utilisateur sont modifiés pour accorder les privilèges root.

#### Résultats :

- Aucun bug ou crash détecté.
- L'élévation des privilèges n'a pas fonctionné, malgré un code fonctionnel et sans erreur apparente. Les raisons peuvent être liées à des incompatibilités ou des limitations kernel.

---

### 3. **Cacher un utilisateur**

Ce module visait à dissimuler un utilisateur spécifique du système. L'objectif était de rendre invisible l'utilisateur dans des contextes tels que :

- La commande `cat /etc/passwd`
- Les résultats des appels système impliquant l'utilisateur.

#### Fonctionnalités principales :

- **Manipulation de `getdents64`** : Le module tente de filtrer les entrées du répertoire utilisateur pour exclure un utilisateur spécifique.
- **Interaction avec les structures kernel** : La manipulation des structures de répertoires est au cœur de cette tentative.

#### Résultats :

- Le code a été partiellement implémenté, mais n'a pas été finalisé par manque de temps.
- Aucun test concluant n'a été réalisé.

---

## Outils et Techniques Utilisés

- **Kprobes** : Utilisés pour intercepter les appels système tels que `kill` et `open`.
- **Manipulation des Credentials** : Modification des credentials de processus pour accorder les privilèges root.
- **Hooks Kernel** : Implémentation de hooks pour manipuler le comportement des appels système.
- **Structures du Kernel** : Exploration des structures kernel liées aux utilisateurs et aux fichiers.

---
