"use client"
import Header from "src/components/Header/Header";
import StripeComponent, { stripePromise } from "src/components/stripeComponent";
import { Button } from "src/components/ui/button";
import { Elements } from "@stripe/react-stripe-js";
import { useParams, useSearchParams } from 'next/navigation'
import { Suspense, useEffect, useState } from "react";

function convertToSubcurrency(amount: number, factor = 1000) {
	return Math.round(amount * factor);
}

function GetSearchParams() {
	const params = useSearchParams();	
	return params;
}

export default function StripePayment() {
	const inputParams = GetSearchParams()
	const [queryParams, setQueryParams] = useState<{
		cost: number,
		url_file: string,
		username: string
	}>();

	const { cost, url_file, username }: { cost: number; url_file: any; username: string } = useParams(); // Parse values from the URL
	useEffect(() => {
		// const cost = parseInt(inputParams.get('cost'),10);	

	}, [cost,url_file, username])


	return (
		<Suspense fallback={<div>Loading...</div>}>
		<main>
			<Header/>
			<h2 className="text-center text-bold">
			Paye les frais de compute 

			</h2>

			<form className="flex flex-col items-left" >
				<Elements
					options={{
						mode: "payment",
						amount: 100,
						currency: "eur"
					}}
					stripe={stripePromise}
				>

<StripeComponent amount={100} url_file={url_file} username={username} />
</Elements>
			</form>
		</main>
		</Suspense>
	);
}
