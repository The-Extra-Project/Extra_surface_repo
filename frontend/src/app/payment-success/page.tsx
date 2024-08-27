"use client"
// import { useEffect } from "react";
import Header from "src/components/Header/Header";
import { useSearchParams } from "next/navigation";
import { useState, useEffect } from "react";
import { Suspense } from "react";
import { configDotenv } from "dotenv";
import { resolve } from "path";
import {Alert, AlertTitle, AlertDescription} from "src/components/ui/alert"
import {env} from "@/env"


function useGetAllSearchParams() {
	const searchParams = useSearchParams();
	const params: { [anyProp: string]: string } = {};
  
	searchParams.forEach((value, key) => {
	  params[key] = value;
	});
	return params;
  };



 
export default function PaymentSuccess() {

    const [amount, getAmount] = useState(0);
    const [urlFile, getUrlFile] = useState();

    const getParams = useGetAllSearchParams();
    const params: number = parseInt(getParams["amount"], 10);
    const tiles_file = getParams["url_file"] as string
    const email = getParams["username"]


    useEffect(() => {
      getAmount(params);

      
      let schedule_data_input = JSON.stringify({
        "input_url": tiles_file,
        "username": email 
      });


       fetch( env.NEXT_PUBLIC_API_SERVER_URL +  "/email/send_mail_paid?receiever_email=" + email + "&payment_reference=" +  amount + "&file_details=" +tiles_file ,  {
        method: "POST",
        body: ''
      });


      //TODO: calling the api for the reconstruction call
       fetch( env.NEXT_PUBLIC_API_SERVER_URL!  +'/reconstruction/schedule',  {
              method: 'POST',
              headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json',
              },       
              body: schedule_data_input
            });
          },
    [params,tiles_file,email]
  );
      return (
        <Suspense fallback={<div>Loading...</div>}>
      <main className="max-w-6xl mx-auto p-10 text-black text-center border m-10 rounded-md bg-gradient-to-tr">
       <Header/>
        <div className="mb-10">
          <h1 className="text-4xl font-extrabold mb-2"> Fecilitation, on viens de recevoir le paiment de montant</h1>
          <div className=" text-2xl font-bold">
            ${amount} 
          </div>
          <div>
        <div className="text-xl font-bold">
          Pour le reconstruction de tuilles:- {tiles_file}
        </div>
          Vous receverez tres prochainment le mail pour presente le prochain steps.
          </div>
        </div>
        <Alert>
	<AlertTitle>
		Attention !
	</AlertTitle>
	<AlertDescription>
	Selon usage de la plateforme, prévoir 3 à 48h de attente pour recevoir les données.
	</AlertDescription>
</Alert>
      </main>
      </Suspense>
    );
  }