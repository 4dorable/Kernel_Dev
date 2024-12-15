FROM alpine:latest

# Installer openrc pour permettre un démarrage correct
RUN apk add --no-cache openrc

# Copier le module rootkit dans le dossier approprié
COPY ./modules /lib
