"use client"
import Header from "src/components/Header/Header";
// import StripeComponent from "src/components/stripeComponent";
import { Button } from "src/components/ui/button";
import {loadStripe} from "@stripe/stripe-js"
import { Elements, useElements, useStripe, PaymentElement } from "@stripe/react-stripe-js";
import {  useSearchParams } from 'next/navigation'
import React, { Suspense, useEffect, useState, useRef } from "react";
import { useRouter } from "next/navigation";

import {env} from "@/env"

const stripePromise = loadStripe( env.NEXT_PUBLIC_STRIPE_PUBLIC_KEY!);

function convertToSubcurrency(amount: number, factor = 1000) {
	return Math.round(amount * factor);
}
/**
 * fetches all the params being passed in the router library (credits to the stackoverflow):https://stackoverflow.com/questions/78330149/nextjs-app-router-get-all-query-parameters-in-string 
 * @returns  gets the dictionary of all the params
 */
function useGetAllSearchParams() {
	const searchParams = useSearchParams();
	const params: { [anyProp: string]: string } = {};
  
	searchParams.forEach((value, key) => {
	  params[key] = value;
	});
	return params;
  };

export default function StripePayment() {
	const inputParams = useGetAllSearchParams();
	 const amount = parseInt(inputParams["cost"],10);
	 const url_file: File = inputParams["url_file"] as  unknown as File ;
	 const username = inputParams["username"];

	return (
		<main>
			<Header/>
			{ 
			<Elements
			        stripe={stripePromise}
					options={{
					  mode: "payment",
					  amount: convertToSubcurrency(amount),
					  currency: "eur",
					}}
			>
				<WrapperElement amount={amount} url_file={url_file} username={username}/>
					</Elements>
}
		</main>

	);
}

function WrapperElement({amount, url_file, username})  {
	const elements = useElements()
	const stripe = useStripe()
	const router = useRouter()
	const [clientSecret, setCS] = useState("");


	useEffect(() => {
		setLoading(true);
		if (amount)
			{
			 fetch("/api/payment", {
				method: "POST",
				headers: {
					"Content-Type": "application/json",
				  },
				  body: JSON.stringify({ amount: convertToSubcurrency(amount) }),
			})
			.then((res) => res.json())
			.then((data) => {
				 setCS(data.clientSecret)
			});
		}
		setLoading(false)

	}, [])
	
	
	const [errorMessage, setErrorMessage] = useState<string>();
	const [loading, setLoading] = useState(false);
	
	const handlePaymentSuccess = (amount,url_file, username) => {
		router.replace(env.NEXT_PUBLIC_WEBSITE_URL +`/payment-success?amount=${amount}&url_file=${url_file}&username=${username}`);
	};
	
	const paid = useRef(amount)

	
	const handleSubmit = async () => {
		if (isNaN(amount) || amount <= 0) {
			return <div>Error: Invalid amount value</div>;
		}
			
		if (!stripe || !elements) {
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
			  return_url: env.NEXT_PUBLIC_WEBSITE_URL +`/payment-success?amount=${paid.current}&url_file=${url_file}&username=${username}`,
			},
		  });
		  if (error) {
			// This point is only reached if there's an immediate error when
			// confirming the payment. Show the error to your customer (for example, payment details incomplete)
			setErrorMessage(error.message);
		  } 
		  else {
			handlePaymentSuccess(paid.current, url_file,username)
		  }
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
		<>
		{clientSecret && <PaymentElement />}
		{errorMessage && <div>{errorMessage}</div>}
			<form onSubmit={handleSubmit} className="flex flex-col items-left" >
			<button
        disabled={!stripe || loading}
        className="text-white w-full p-5 bg-black mt-2 rounded-md font-bold disabled:opacity-50 disabled:animate-pulse"
		onClick={() => handlePaymentSuccess(amount, url_file,username) }
      >
        {!loading ? `Pay $${amount}` : "Processing..."}
      </button>
			</form>

	</>
	)
}
