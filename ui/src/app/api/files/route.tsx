"use server"
import fs from "fs/promises"
import { NextRequest } from "next/server";
import {resolve} from "path"
import { supabaseClient } from "src/utils/supabase_server";
import { Database, Tables } from "src/utils/types_db";

export async function POST(req:NextRequest) {
    const {file} : {file: File} = await req.json()
    try {
    const fileBuffer = await file.arrayBuffer()
    const buffer = new Uint8Array(fileBuffer);
    var storage_directory = resolve(__dirname, "..", "..", "..","..", "datas")
    console.info("storing the result in" + (storage_directory))
    await fs.writeFile(  storage_directory + `/${file[0].name}`, buffer);
    }
    catch(Error) {
        console.error("/api/files not working: " + Error)
    }
}