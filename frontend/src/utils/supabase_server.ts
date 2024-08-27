
//import { createServerClient,type CookieOptions } from "@supabase/ssr";
//import {cookies} from "next/headers"
import { Database } from "./types_db";

import { configDotenv } from "dotenv";

import { createClient } from '@supabase/supabase-js'
import { resolve } from "path";

import { env } from "@/env";

// Create a single supabase client for interacting with your database
export const supabaseClient = createClient<Database>(env.SUPABASE_URL!, env.SUPABASE_ANON_KEY!)