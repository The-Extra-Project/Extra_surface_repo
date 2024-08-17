#!/bin/sh

cp .env.frontend  frontend/.env
cp .env.backend backend/api/.env
cp .env.sparkling_washeur backend/sparkling_washeur/.env

docker compose build 
docker compose up -d 