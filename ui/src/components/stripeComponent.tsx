"use client"
import { Elements, useElements, useStripe, PaymentElement } from "@stripe/react-stripe-js";
import { loadStripe } from "@stripe/stripe-js";
import { configDotenv } from "dotenv";
import React, { useEffect, useState } from "react";
import path, { basename, resolve } from "path";
import { useRouter } from "next/router";

configDotenv(
    {
        path: "../../.env"
    }
)
// () => {
//     try {

//         configDotenv({
//             path: path.join('.', '../', '.env')
//         })

//     }

//     catch (Error) {
//         console.error("error in logging in the Configuration file: " + Error)
//     }

// }

export const stripePromise = loadStripe(process.env.STRIPE_PUBLIC_KEY);

export default function StripeComponent({ amount }: { amount: number }) {
    const stripe = useStripe();
    const elements = useElements();
    const [errorMessage, setErrorMessage] = useState<string>();
    const [loading, setLoading] = useState(false);
    const [clientSecret, setClientSec] = useState("");
    const [amt, setAmt] = useState(0);

    const handleSubmit = async () => {
        

        setLoading(true);
        
        setAmt(amount)
        
         fetch("/api/payment", {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify({ amount: amt }),
        }).catch((data) => (data.json()))
        .then((data) => setClientSec(data.clientSecret))
          


        if (!stripe || !elements) {
            setErrorMessage("Stripe.js has not loaded yet.");
            setLoading(false);
            return;
        }

        const paymentElement = elements.getElement(PaymentElement);
        if (!paymentElement) {
            setErrorMessage("Payment Element is not mounted.");
            setLoading(false);
            return;
        }

        const { error: submitError } = await elements.submit();

        if (submitError) {
            setErrorMessage(submitError.message);
            setLoading(false);
            return;
        }

        const { error } = await stripe.confirmPayment({
            elements,
            clientSecret,
            confirmParams: {
                return_url: `http://www.localhost:3000/payment-success?amount=${amount}`,
            },
        });
        if (error) {
            setErrorMessage(error.message);
        }
        setLoading(false);    
    };

    if (!stripe || !elements) {
        return (
            <div className="flex items-center justify-center">
                <div
                    className="inline-block h-8 w-8 animate-spin rounded-full border-4 border-solid border-current border-e-transparent align-[-0.125em] text-surface motion-reduce:animate-[spin_1.5s_linear_infinite] dark:text-white"
                    role="status"
                >
                    <span className="!absolute !-m-px !h-px !w-px !overflow-hidden !whitespace-nowrap !border-0 !p-0 ![clip:rect(0,0,0,0)]">
                        Loading...
                    </span>
                </div>
            </div>
        );

    }

    return (
        <>

             <PaymentElement />
            <button
                disabled={!stripe}
                className="text-white w-full p-5 bg-black mt-2 rounded-md font-bold disabled:opacity-50 disabled:animate-pulse"
                onClick={handleSubmit}
            >
                {!loading ? `Pay $${amount}` : "Processing..."}
            </button>
            </>
    );
}