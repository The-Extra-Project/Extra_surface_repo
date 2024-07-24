import { Elements, useElements, useStripe, PaymentElement } from "@stripe/react-stripe-js";
import { loadStripe } from "@stripe/stripe-js";
import { configDotenv } from "dotenv";
import React, { useEffect, useState } from "react";
import path from "path";
import { env } from "src/env";

export const stripePromise = loadStripe("pk_test_51PGIH1RxsgXWpWf561hoKKpNV4gHBox0jscPYZ5pgj6LSKn4TP14q8xv9tkW9EN59QoDatNB7S1FBBgSQ1C5HDXH00YtHhR2mm");

export default function StripeComponent({ amount, email, URLs}: { amount: number, email: string, URLs: string[] }) {
    const stripe = useStripe();
    const elements = useElements();
    const [errorMessage, setErrorMessage] = useState<string>();
    const [loading, setLoading] = useState(false);
    const [clientSecret, setClientSec] = useState("");
    
    useEffect(() => {
         fetch("/api/payment", {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify({ amount: amount }),
        })
        .then((res) => res.json())
        .then((data) => setClientSec(data.clientSecret));
    
    }, [amount]);

    const handleSubmit = async (event: React.FormEvent<HTMLFormElement>) => {
        event.preventDefault();
        setLoading(true);
    
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
                return_url: `http://www.localhost:3000/payment-success?amount=${amount}&email=${email}&URLs=${URLs}`,
            },
        });
        if (error) {
            setErrorMessage(error.message);
        }
        setLoading(false);
    };
    
    if (!clientSecret || !stripe || !elements) {
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

            <form onSubmit={handleSubmit} className="bg-white p-2 rounded-md">
                {clientSecret && <PaymentElement />}
                {errorMessage && <div>{errorMessage}</div>}
                <button
                    disabled={!stripe}
                    className="text-white w-full p-5 bg-black mt-2 rounded-md font-bold disabled:opacity-50 disabled:animate-pulse"
                >
                    {!loading ? `Pay $${amount}` : "Processing..."}
                </button>
            </form>
    );
}