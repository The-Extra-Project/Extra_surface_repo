import { Database, Tables } from 'src/utils/types_db';
import {stripe} from "src/utils/stripe/config";
import {SupabaseClient} from "@supabase/supabase-js"
import { cache } from 'react';
import {supabaseClient} from "src/utils/supabase_server"
type JobOrder = Tables<'selectionjob'>
type ExtraSurface = Tables<'extra_surface'>
type Payment = Tables<'payment'>



export const getUser = cache(async (supabase: SupabaseClient<Database>) => {
  const {
    data: { user }
  } = await supabase.auth.getUser();
  return user;
});


export const getPaymentOrder = cache(async (supabase :SupabaseClient<Database>) => {
    let user = getUser(supabaseClient)
    const {data: any, error} = await supabase.from("extra_surface").select('*').eq('selection_job_id', (await user).id?.[0])
    .order('payment_id', { ascending: false })
    .limit(1);
    });

