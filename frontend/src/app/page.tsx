"use client";
import Image from "next/image";
import Link from "next/link";
import Header from "src/components/Header/Header";
import fs from "fs/promises";
//import EmbedGraph from "src/components/embed_graph";
import embed_graph from "src/public/Card_France.png";
import Popup from "reactjs-popup"
import {Alert, AlertDescription, AlertTitle }  from "src/components/ui/alert"
import { Button } from "src/components/ui/button";
import { useState, useRef} from "react";
import {
	FileUploader,
	FileUploaderContent,
	FileUploaderItem,
	FileInput,
} from "src/components/ui/FileUploader";
import { Paperclip } from "lucide-react";
import { configDotenv } from "dotenv";

import { useRouter } from "next/navigation";
import { resolve } from "path";
//import { getUser } from "src/utils/supabase_queries";

//import { env } from "env";

//const resend = new Resend(env.NEXT_PUBLIC_RESEND_API_KEY || "");


export default function Home() {

	const FileSvgDraw = () => {
		return (
			<>
				<svg
					className="w-8 h-8 mb-3"
					aria-hidden="true"
					xmlns="http://www.w3.org/2000/svg"
					fill="none"
					viewBox="0 0 20 16"
				>
					<path
						stroke="currentColor"
						strokeLinecap="round"
						strokeLinejoin="round"
						strokeWidth="2"
						d="M13 13h3a3 3 0 0 0 0-6h-.025A5.56 5.56 0 0 0 16 6.5 5.5 5.5 0 0 0 5.207 5.021C5.137 5.017 5.071 5 5 5a4 4 0 0 0 0 8h2.167M10 15V6m0 0L8 8m2-2 2 2"
					/>
				</svg>
				<p className="text-xs">Charger fichier liste_dalle.txt</p>
			</>
		);
	};
	const router = useRouter()
	const [upload, setUpload] = useState(false);
	const [payment, setPayment] = useState(false);
	const [portal, showPortal] = useState(false);
	const [files, setFiles] = useState<File[]>([]);
	const [invalidFile, setInvalidFile] = useState(false);
	const [URLs, setURLs] = useState<string[]>([]);
	const [cost, setCost] = useState(0);
	const [email, setEmail] = useState("");
	const dropZoneConfig = {
		maxFiles: 1,
		maxSize: 1024 * 1024 * 1,
		multiple: true,
		accept: {
			"text/plain": [".txt"],
		},
	};

	const onSubmit = async (event: React.MouseEvent<HTMLButtonElement, MouseEvent>) => {
		event.preventDefault();

		showPortal(true)
		router.push(`/payment_submission?cost=${cost}&url_file=${files[0]}&username=${email}`)
		const formData = new FormData();
		formData.append("file", files[0]);
		formData.append("email",email )
		
		fetch("/api/files", {
		  method: "POST",
		  body: formData,
		})  
  
	
	
	};
	//const form = useForm();
	const onUpload = (file: File) => {
		const reader = new FileReader();
		const cosntPrice = 5;
		let totalPrice = 0;
		reader.onload = (event) => {
			const content = event.target?.result as string;
			const urls = content.split("\n").filter((url) => url.trim() !== "");
			let urlInvalidLines = []
			let invalidFile = false
			let desired_file_pattern = /^(http|https):\/\/[^ "]+$/;

			for (let i = 0; i < urls.length; i++) {
				if(
					desired_file_pattern.test(urls[i])
				) 
				{
					totalPrice += cosntPrice; 

				}
				else
				{
					urlInvalidLines.push(i)
					invalidFile = true

				}
			}
			if (invalidFile) {
				setInvalidFile(true)
			}
			else 
			{
				setInvalidFile(false); 

			}

			setCost(totalPrice);
			setURLs(urls);
			setUpload(true);
      setFiles([file])
		};

		reader.readAsText(file);
	};

	return (
		<>
			<section className="">
				<Header />
				<div
					className="container"
					style={{ display: "flex", flexWrap: "wrap" }}
				>
					<div className="title-container">
						<h1 className="text-2xl font-bold py-2">
							Passer commande de reconstruction 3D issues de LidarHD
						</h1>
						<text className="text-grey-400">
							Première étape d&apos;un projet dédié à la mise à jour des données de
							carte 3D, en savoir plus{" "}
							{
								<Link className="font-bold" href="www.extralabs.xyz/fr/project">
									ici.
								</Link>
							}
						</text>
					</div>
					<div className="image-container">
						<p className="font-medium pb-2">
							1. Se rendre sur le site de{" "}
						<Link
						className="underline"
						href={"https://diffusion-lidarhd.ign.fr"}
					>
						diffusion-lidarhd 
					</Link>
					{" "}
							pour obtenir la liste des dalles que vous souhaitez reconstruire
						</p>
					
						<Image
						src={embed_graph}
						style={{ border: "black solid 1px" }}
						alt="graph"
						onClick={
							() => window.open("https://diffusion-lidarhd.ign.fr", "_blank").focus()
						}
						/>
					</div>
					<div className="form-container">
						<form className="flex flex-col items-left">
							<div>
								<p className="font-medium">
									2. Charger la liste des nuages de points obtenue (cliquer ou
									glisser-déposer)
								</p>
								<FileUploader
									value={files}
									onValueChange={(newFiles) => {
										setFiles(newFiles);
										if (newFiles && newFiles.length > 0) {
											onUpload(newFiles[0]);
										}
									}}
									dropzoneOptions={dropZoneConfig}
									className="relative p-2"
								>
									<FileInput className="outline-1 outline">
										<div className="flex items-center justify-center flex-col pt-3 pb-4 w-full ">
											<FileSvgDraw />
										</div>
									</FileInput>
									<FileUploaderContent>
										{files &&
											files.length > 0 &&
											files.map((file, i) => (
												<FileUploaderItem key={i} index={i}>
													<Paperclip className="h-4 w-4 stroke-current" />
													<span>{file.name}</span>
																									
													</FileUploaderItem>
												// for each dalle file corresponding
											))}
									</FileUploaderContent>
								</FileUploader>
							</div>
							<div className="py-">
								<p className="font-medium py-2">
									3. Renseigner votre adresse e-mail pour recevoir les données
								</p>
								<input
									type="email"
									id="email"
									name="email"
									required
									placeholder="E-mail"
									className="outline outline-1 rounded-sm w-full p-2"
									style={{ width: "-webkit-fill-available" }}
									value={email}
									onChange={(e) => setEmail(e.target.value)}
								/>
							</div>

							<p className="font-medium py-3">
								Nombre de tuiles sélectionnées: { !invalidFile && <text>{URLs.length}</text>}
								<br />
								Prix: {!invalidFile &&  URLs.length * 5}€
							</p>
							<p className="text-xs">
								Ce prix nous permet de payer les coûts de calcul et de financer
								le travail de notre équipe.
							</p>
							{
								invalidFile &&
								<Alert>
									<AlertTitle>
										file is invalid: refresh and upload again.
									</AlertTitle>
								</Alert>
							}
							
							{
							<Button
								type="submit"
								className="rounded-sm my-10 py-6"
								onClick={onSubmit}
								style={{ backgroundColor: "#589EA5" }}
								disabled={invalidFile}
							>
								{" "}
								Lancer le téléchargement
							</Button>
							}
							{
								portal &&
								<Alert>
									<AlertTitle>
										Now starting the stripe payment
									</AlertTitle>
								</Alert>
							}
							
						</form>
					</div>
				</div>
			</section>
		</>
	);
}

