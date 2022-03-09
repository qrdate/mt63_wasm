const fs = require( 'fs' )

const mt63 = require( "../build/mt63Wasm" );

mt63().then( module =>
{
	const mt63EncodeString = module.cwrap( 'encodeString', 'number', [ 'string', 'number', 'number', 'number' ] );
	const mt63GetBuffer = module.cwrap( 'getBuffer', 'number' );

	let buff_size = mt63EncodeString( "MT63 Test!", 2000, 2000, 1 );
	let buff_offset = mt63GetBuffer();

	let data = module.HEAPF32.subarray( buff_offset / 4, buff_offset / 4 + buff_size );

	fs.writeFileSync( './encoded.raw', data );
} );
