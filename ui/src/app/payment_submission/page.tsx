"use client"
import Header from "src/components/Header/Header";
import StripeComponent, { stripePromise } from "src/components/stripeComponent";
import { Button } from "src/components/ui/button";
import { Elements } from "@stripe/react-stripe-js";
import { useSearchParams } from 'next/navigation'
import { useEffect, useState } from "react";


function convertToSubcurrency(amount: number, factor = 1000) {
	return Math.round(amount * factor);
}


export default function StripePayment() {
	const inputParams = useSearchParams()

	
	const [queryParams, setQueryParams] = useState(0);


	useEffect(() => {
		const params = parseInt(inputParams.get('cost'),10);	
		setQueryParams(params);
	}, [inputParams])
	

	alert("get query params:" + queryParams)


	return (
		<main>
			<Header />
			<form className="flex flex-col items-left" >
				<Elements
					options={{
						mode: "payment",
						amount: convertToSubcurrency(queryParams),
						currency: "eur"
					}}
					stripe={stripePromise}
				>
					<StripeComponent amount={queryParams} />
				</Elements>
			</form>
		</main>
	);
}
