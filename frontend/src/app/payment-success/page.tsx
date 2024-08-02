"use client"
// import { useEffect } from "react";
import Header from "src/components/Header/Header";
import { useSearchParams } from "next/navigation";
import { useState, useEffect } from "react";
import { Suspense } from "react";


function UseSearchParams() {
  const getParams = useSearchParams();
  return [ getParams]
}
 
export default function PaymentSuccess() {

    const [amount, getAmount] = useState(0);
    const [urlFile, getUrlFile] = useState();



    useEffect(() => {
      const [getParams] = UseSearchParams()
      const params: number = parseInt(getParams.get("amount"), 10);
    
      getAmount(params);

   
      //TODO: calling the api for the reconstruction call
      const response =  fetch('https://localhost:8000/reconstruction/post',  {
              method: 'POST',
              headers: {
                'Content-Type': 'application/json',
              },
              body: JSON.stringify({
                params
              }),
            });
      // call the schedule function 
    //   sendEmail();
    });
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
          Pour le reconstruction de tuilles ${urlFile}
        </div>
          Vous receverez tres prochainment le mail pour presente le prochain steps.
          </div>
         
        </div>
      </main>
      </Suspense>
    );
  }