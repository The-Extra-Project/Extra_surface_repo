"use server"
import fs from "fs/promises"

import { NextRequest } from "next/server";

export async function POST(req:NextRequest) {
    const {file} : {file: File} = await req.json()

    try {

    const fileBuffer = await file.arrayBuffer()
    const buffer = new Uint8Array(fileBuffer);
    await fs.writeFile(`./${file[0].name}`, buffer);

    }
    catch(Error) {
        console.error("/api/files not working: " + Error)
    }

}
