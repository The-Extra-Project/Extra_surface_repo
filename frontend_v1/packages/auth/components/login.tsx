//import { SignIn as ClerkSignIn } from '@clerk/nextjs';
import { Amplify } from "aws-amplify";
import { authConfig } from "../config/exports";
import {Authenticator} from "@aws-amplify/ui-react"
import {createServerRunner} from "@aws-amplify/adapter-nextjs"
import '@aws-amplify/ui-react/styles.css'
import { cookies } from "next/headers";
import {  } from "@repo/design-system";



const serverRunner = createServerRunner({config:authConfig})
export const SignIn = () => (
<Authenticator hideSignUp={true}>    
</Authenticator>
)
