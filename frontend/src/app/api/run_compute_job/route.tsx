"use server"
import { NextRequest, NextResponse } from "next/server";
import { exec } from "child_process"; 

export async function POST(request: NextRequest) {

try {
    const {filepath, email} = await request.json();
    // calling the previous recursive api
    const response = await fetch('https://localhost:8000/reconstruction/post', {
        body: JSON.stringify(
            {
                filepath, email
            }
        )
    })
    return await request.json()
}

catch(error) {

    console.error("/api/run_compute_job" + error)
}



}