
//import { createServerClient,type CookieOptions } from "@supabase/ssr";
//import {cookies} from "next/headers"
import { Database } from "./types_db";

import { configDotenv } from "dotenv";

import { createClient } from '@supabase/supabase-js'

// Create a single supabase client for interacting with your database
export const supabaseClient = createClient<Database>(process.env.NEXT_SUPABASE_PUBLIC_KEY!, process.env.NEXT_PUBLIC_SUPABASE_ANON_KEY!)

