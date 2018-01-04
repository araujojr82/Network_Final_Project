#include "Salt.h"

#define SALT_SIZE 64

void MyHandleError( LPTSTR psz )
{
	_ftprintf( stderr, TEXT( "An error occurred in the program. \n" ) );
	_ftprintf( stderr, TEXT( "%s\n" ), psz );
	_ftprintf( stderr, TEXT( "Error number %x.\n" ), GetLastError() );
	_ftprintf( stderr, TEXT( "Program terminating. \n" ) );
	exit( 1 );
} // End of MyHandleError

void generateSalt( std::string &salted )
{
	HCRYPTPROV hCryptProv;  // handles to a Cryptographic Service Providers

	// Get a handle to the default PROV_RSA_FULL provider.
	if( CryptAcquireContext( &hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )
	{
		_tprintf( TEXT( "CryptAcquireContext succeeded.\n" ) );
	}
	else
	{
		if( GetLastError() == NTE_BAD_KEYSET )
		{
			// No default container was found. Attempt to create it.
			if( CryptAcquireContext( &hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET ) )
			{
				_tprintf( TEXT( "CryptAcquireContext succeeded.\n" ) );
			}
			else
			{
				MyHandleError( TEXT( "Could not create the default " ) TEXT( "key container.\n" ) );
			}
		}
		else
		{
			MyHandleError( TEXT( "A general error running " ) TEXT( "CryptAcquireContext." ) );
		}
	}

	BYTE    saltData[SALT_SIZE];

	// Generate a random initialization vector
	if( CryptGenRandom( hCryptProv, SALT_SIZE, saltData ) )
	{
		printf( "Random sequence generated. \n" );
		std::string strCasted( reinterpret_cast< char const* >( saltData ), SALT_SIZE );
		salted = strCasted;
	}
	else
	{
		printf( "Error during CryptGenRandom.\n" );
		exit( 1 );
	}
}