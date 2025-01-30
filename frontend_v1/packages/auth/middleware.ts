import { Auth } from 'aws-amplify';
import { NextMiddleware, NextRequest, NextResponse } from 'next/server';

export async function authMiddleware(request: NextRequest): Promise<NextResponse> {
  try {
    const user = await Auth.currentAuthenticatedUser();
    if (user) {
      // User is authenticated, proceed with the request
      return NextResponse.next();
    }
  } catch (error) {
    // User is not authenticated, redirect to sign-in page
    return NextResponse.redirect('/sign-in');
  }
}

export const config = {
  matcher: [
    // Match all request paths except for the ones starting with:
    // - _next/static (static files)
    // - _next/image (image optimization files)
    // - favicon.ico (favicon file)
    // Feel free to modify this pattern to include more paths.
    '/((?!_next/static|_next/image|favicon.ico).*)',
  ],
};