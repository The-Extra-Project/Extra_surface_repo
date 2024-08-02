#!/bin/sh
function ipfs_run() {
ipfs daemon &
}

function ipfs_push_file() {
    upload_folder_path = $1
    ipfs add -r ${upload_folder_path}
}

@#