import type { Metadata } from "next";
import { Inter } from "next/font/google";
import "./globals.css";
import { cn } from "src/utils";
import { ThemeProvider } from "src/components/theme-providers";

const inter = Inter({
  subsets: ["latin"],
  variable: "--font-sans",
});

export const metadata: Metadata = {
  title: "Extra surface",
  description: "Extra surface demo application ",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="fr" suppressHydrationWarning>
      <body className={cn(
          "min-h-screen bg-background bg-gray-200 font-sans antialiased",
          inter.variable,
        )}>
     <ThemeProvider attribute="class" defaultTheme="system" enableSystem>
          {children}
        </ThemeProvider>
    </body>
    </html>
  );
}
