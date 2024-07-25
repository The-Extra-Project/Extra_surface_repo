"use client"
import Header from "src/components/Header/Header";
import StripeComponent, { stripePromise } from "src/components/stripeComponent";
import { Button } from "src/components/ui/button";
import { Elements } from "@stripe/react-stripe-js";

function convertToSubcurrency(amount: number, factor = 1000) {
	return Math.round(amount * factor);
}


export default function StripePayment(amount: any) {

	return (

		<main className="max-w-6xl mx-auto p-10 text-black text-center border m-10 rounded-md bg-gradient-to-tr">

			<Header />
			<form className="flex flex-col items-left" >
				<Elements
					options={{
						mode: "payment",
						amount: convertToSubcurrency(amount),
						currency: "eur"
					}}
					stripe={stripePromise}
				>
					<StripeComponent amount={amount} />
				</Elements>
				<Button type="submit" > Pay Via Skype</Button>

			</form>
		</main>

	);




}

