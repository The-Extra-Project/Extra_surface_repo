#!/bin/sh
mkdir -p storage/s3/
chmod -R 777 storage/s3/

sudo s3fs extra-protocol-lidarhd storage/s3/ -o allow_other -o default_acl=public-read
