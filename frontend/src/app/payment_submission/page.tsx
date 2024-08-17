"use client"
import Header from "src/components/Header/Header";
import StripeComponent from "src/components/stripeComponent";
import { Button } from "src/components/ui/button";
import { Elements } from "@stripe/react-stripe-js";
import { useParams, useSearchParams } from 'next/navigation'
import React, { Suspense } from "react";
import { stripePromise } from "src/components/stripeComponent";

function convertToSubcurrency(amount: number, factor = 1) {
	return Math.round(amount * factor);
}

function useAmount() {
	const searchParams = useSearchParams();	

	return convertToSubcurrency(+searchParams.get('cost'));
}


export default function StripePayment() {
	const inputParams = useSearchParams()
	const cost = inputParams.get("cost");
	const amount = useAmount();
	const url_file = inputParams.get("url_file");
	const username = inputParams.get("username");
	return (
		<main>
		<Suspense fallback={<div>Loading...</div>}>
			<Header/>
			<form className="flex flex-col items-left" >
			
<StripeComponent amount={Math.round(parseInt(cost,10))} url_file={url_file} username={username} />
			</form>
			</Suspense>
		</main>
	);
}
