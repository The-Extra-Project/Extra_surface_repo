//import {} from "@repo/design-system"
import Link from "next/link"
import {env} from "@repo/env"
import {cn} from "@repo/design-system/lib/utils"
import { buttonVariants } from "@repo/design-system/components/ui/button"

export default async function IndexPage() {
    return (
        <>
        <section className="space-y-6 pb-8 pt-6 md:pb-12 md:pt-10 lg:py-32">
        <h1 className="font-heading text-3xl sm:text-5xl md:text-6xl lg:text-7xl">
            Application to 3D reconstruct watertight large pointcloud dataset. 
        </h1>
        <div className="space-x-4">
            <Link href="/login" className={cn(buttonVariants({ size: "lg" }))}>
              Get Started
            </Link>
            <Link
              href={""}
              target="_blank"
              rel="noreferrer"
              className={cn(buttonVariants({ variant: "outline", size: "lg" }))}
            >
              GitHub
            </Link>
          </div>
        </section>
        <section
        id="features"
        className="container space-y-6 bg-slate-50 py-8 dark:bg-transparent md:py-12 lg:py-24"
      >
      </section>
        
        </>
    )
}