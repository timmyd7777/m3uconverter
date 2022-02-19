// This program converts .m3u playlist files exported from Apple Music to the .m3u format
// needed by the SanDisk Clip family of MP3 players. Specifically, it removes path information
// from audio file names, and saves the files in Windows (CRLF) text format. The input files
// are overwritten with their converted versions.
//
// Compile this prgram with: g++ -o m3uconverter m3uconverter.cpp
// To run the compiled program on a folder full of .m3u files: ./m3uconverter *.m3u

#include <cstdio>
#include <string>
#include <unistd.h>

using namespace std;

// Tests whether a string (str) ends with another string (suffix)
// Adapted from https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c

bool endsWith ( const string &str, const string &suffix )
{
    return str.size() >= suffix.size() && str.compare ( str.size() - suffix.size(), suffix.size(), suffix ) == 0;
}

// Wrapper for fgets() that reads into a C++ string (line)
// from a C FILE pointer (file) opened for reading in text mode.
// Discards any terminating newline to emulate behavior of C++ getline().
// Returns true if successful or false on failure (end-of-file, etc.)

bool fgetline ( FILE *file, string &line )
{
	line = "";
	
	while ( true )
	{
		int c = fgetc ( file );
		if ( c == EOF )
			return false;
		
		if ( c == '\r' || c == '\n' )
			return true;
			
		line += c;
	}
}

int main ( int argc, char *argv[] )
{
	for ( int i = 1; i < argc; i++ )
	{
		// Skip files whose names don't end with ".m3u"
		
		string infilename ( argv[i] );
		if ( ! endsWith ( infilename, ".m3u" ) )
		{
			fprintf ( stderr, "Skipping %s.\n", argv[i] );
			continue;
		}
		
		// Open input and output file; continue on failure
		
		FILE *infile = fopen ( argv[i], "rb" );
		if ( infile == NULL )
		{
			fprintf ( stderr, "Cam't open %s\n", argv[i] );
			continue;
		}
		
		string outfilename = infilename + ".converted";
		FILE *outfile = fopen ( outfilename.c_str(), "wb" );
		if ( outfile == NULL )
		{
			fprintf ( stderr, "Cam't open %s\n", outfilename.c_str() );
			fclose ( infile );
			continue;
		}
		
		// Convert input file line-by-line to output file.
		
		string line;
		bool good = true;
		while ( fgetline ( infile, line ) )
		{
			// Skip empty lines
			
			if ( line.length() == 0 )
				continue;
				
			// If input line doesn't begin with "#EXT", assume it's an audio file path
			// and strip path prefix.
			
			if ( line.compare ( 0, 4, "#EXT" ) != 0 )
			{
				size_t pos = line.find_last_of ( "//" );
				if ( pos != string::npos )
					line = line.substr ( pos + 1, string::npos );
			}
		
			// Write line to output file with CRLF (Windows) line ending.
			
			line += "\r\n";
			if ( fwrite ( line.c_str(), line.length(), 1, outfile ) != 1 )
			{
				fprintf ( stderr, "Can't write output line %s\n", line.c_str() );
				good = false;
				break;
			}
		}
		
		// Close input and output files
		
		fclose ( infile );
		fclose ( outfile );
		
		// If successful, delete input file and replace with output file.
		
		if ( good )
		{
			unlink ( argv[i] );
			rename ( outfilename.c_str(), infilename.c_str() );
			fprintf ( stderr, "Converted %s.\n", argv[i] );
		}
		else
			fprintf ( stderr, "Failed to convert %s.\n", argv[i] );
	}
	
	return 0;
}
