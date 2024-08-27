"use client"
import {  useElements, useStripe, PaymentElement } from "@stripe/react-stripe-js";
import { loadStripe } from "@stripe/stripe-js";
import { configDotenv, config } from "dotenv";
import { Router } from "lucide-react";
import { useRouter } from "next/navigation";
import React, { Suspense, useEffect, useState } from "react";
import Path from "path"

let configparams  = config()

export default function StripeComponent({ amount, url_file, username }: { amount: number, url_file: string, username: string }) {
    // const stripe = useStripe();
    // const elements = useElements();
    const [errorMessage, setErrorMessage] = useState<string>();
    const [loading, setLoading] = useState(false);
    const [clientSecret, setClientSec] = useState("");
    const [amt, setAmt] = useState(0);

    const handleSubmit = async () => {
        setLoading(true);
        setAmt(amount)

        fetch("/api/email", {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify({ username, url_file }),
        })
        
        fetch("/api/schedule_compute_job", {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify({ "file_path": url_file, "username": username }),
        })
    };

return (
        <>

<div className="mb-10">
          <h1 className="text-4xl text-center font-extrabold mb-2">
             Verification</h1>

          <div className=" text-2xl font-bold">
            ${amount} 
          </div>
          <div>
        <div className="text-xl font-bold">
          Pour le reconstruction de tuilles ${url_file}
        </div>
          Vous receverez tres prochainment le mail pour presente le prochain steps.
          </div>

         
 
        </div>
        <h2>
            <button
                
                className="text-white w-full p-5 bg-black mt-2 rounded-md font-bold disabled:opacity-50 disabled:animate-pulse"
                onClick={handleSubmit}
            >
                {  "Paiment"        
                }
            </button>
            </h2>
            </>      
    );
}