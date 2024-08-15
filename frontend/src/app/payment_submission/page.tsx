"use client"
import Header from "src/components/Header/Header";
import StripeComponent, { stripePromise } from "src/components/stripeComponent";
import { Elements } from "@stripe/react-stripe-js";
import { useSearchParams } from 'next/navigation'
import { Suspense } from "react";

function convertToSubcurrency(amount: number, factor = 1000) {
	return Math.round(amount * factor);
}

function useAmount() {
	const searchParams = useSearchParams();	
	
	return convertToSubcurrency(+searchParams.get('cost'));
}

export default function StripePayment() {
	const amount = useAmount();	

	return (
		<Suspense fallback={<div>Loading...</div>}>
			<main>
				<Header/>
				<form className="flex flex-col items-left" >
					<Elements
						options={{
							mode: "payment",
							amount,
							currency: "eur"
						}}
						stripe={stripePromise}
					>
						<StripeComponent amount={amount} />
					</Elements>
				</form>
			</main>
		</Suspense>
	);
}
