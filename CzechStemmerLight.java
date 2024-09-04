import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.Writer;
import java.nio.charset.StandardCharsets;

/**
 * @author Dolamic Ljiljana  University of Neuchatel
 *
 * Czech stemmer-removes case endings form nouns and adjectives, possessive adj.
 * endings from names
 * and takes care of palatalisation 
 */
public class CzechStemmerLight {
	
	/**
	 * A buffer of the current word being stemmed
	 */
	private StringBuffer sb=new StringBuffer();

	
	/**
	 * Default constructor
	 */
    public CzechStemmerLight(){} // constructor
    
	public String stem(String input){
		
		//
		input=input.toLowerCase();

		//reset string buffer
		sb.delete(0,sb.length());
		sb.insert(0,input);

		// stemming...
		//removes case endings from nouns and adjectives
		removeCase(sb);

		//removes possessive endings from names -ov- and -in-
		removePossessives(sb);

		String result = sb.toString();
		

		return result;
	}
	private void palatalise(StringBuffer buffer){
		int len=buffer.length();
		
		if( buffer.substring( len- 2 ,len).equals("ci")||
		    buffer.substring( len- 2 ,len).equals("ce")||		
		    buffer.substring( len- 2 ,len).equals("\u010di")||  //-či
		    buffer.substring( len- 2 ,len).equals("\u010de")){  //-če
				
		    buffer.replace(len- 2 ,len, "k");
		    return;
		}
		if( buffer.substring( len- 2 ,len).equals("zi")||
		    buffer.substring( len- 2 ,len).equals("ze")||		
		    buffer.substring( len- 2 ,len).equals("\u017ei")||  //-ži
		    buffer.substring( len- 2 ,len).equals("\u017ee")){  //-že
					
		    buffer.replace(len- 2 ,len, "h");
		    return;
		}
		if( buffer.substring( len- 3 ,len).equals("\u010dt\u011b")||  //-čtě
		    buffer.substring( len- 3 ,len).equals("\u010dti")||       //-čti
		    buffer.substring( len- 3 ,len).equals("\u010dt\u00ed")){  //-čtí
						
		    buffer.replace(len- 3 ,len, "ck");
		    return;
		}
		if( buffer.substring( len- 2 ,len).equals("\u0161t\u011b")||  //-ště
		    buffer.substring( len- 2 ,len).equals("\u0161ti")||       //-šti
		    buffer.substring( len- 2 ,len).equals("\u0161t\u00ed")){  //-ští
						
		    buffer.replace(len- 2 ,len, "sk");
		    return;
		}
		buffer.delete( len- 1 , len);
		return;
	}//palatalise
	
	private void removePossessives(StringBuffer buffer) {
		int len=buffer.length();
		
		if( len> 5 ){
			if( buffer.substring( len- 2 ,len).equals("ov")){
				
			    buffer.delete( len- 2 , len);
			    return;
			}
			if( buffer.substring( len-2,len).equals("\u016fv")){ //-ův
			 	
	        	    buffer.delete( len- 2 , len);
		            return;
			}
		        if( buffer.substring( len- 2 ,len).equals("in")){
			 	
			    buffer.delete( len- 1 , len);
			    palatalise(buffer);
			    return;
			}
		}
		return;
	}//removePossessives

	private void removeCase(StringBuffer buffer) {
		int len=buffer.length();
		// 
		if( (len> 7 )&&
		    buffer.substring( len- 5 ,len).equals("atech")){
			
		    buffer.delete( len- 5 , len);
		    return;
		}//len>7
		if( len> 6 ){
		      if(buffer.substring( len- 4 ,len).equals("\u011btem")){   //-ětem
		
		         buffer.delete( len- 3 , len);
		         palatalise(buffer);
		         return;
		      }
		       if(buffer.substring( len- 4 ,len).equals("at\u016fm")){  //-atům
		    	      buffer.delete( len- 4 , len);
		    	      return;
		      }
		      
		}
		if( len> 5 ){
			    if(buffer.substring( len-3,len).equals("ech")|| 
				  buffer.substring( len-3,len).equals("ich")|| 
				  buffer.substring( len-3,len).equals("\u00edch")){ //-ích
				
				  buffer.delete( len-2 , len);
				  palatalise(buffer);
				  return;
				}
		                if(buffer.substring( len-3,len).equals("\u00e9ho")|| //-ého
				   buffer.substring( len-3,len).equals("\u011bmi")||  //-ěmi
				   buffer.substring( len-3,len).equals("emi")||
				   buffer.substring( len-3,len).equals("\u00e9mu")||  //-ému
				   buffer.substring( len-3,len).equals("\u011bte")||  //-ěte
				   buffer.substring( len-3,len).equals("\u011bti")||  //-ěti
				   buffer.substring( len-3,len).equals("iho")||
				   buffer.substring( len-3,len).equals("\u00edho")||  //-ího
				   buffer.substring( len-3,len).equals("\u00edmi")||  //-ími
				   buffer.substring( len-3,len).equals("imu")){
				
				   buffer.delete( len- 2 , len);
				   palatalise(buffer);
				   return;
			        }
		                if( buffer.substring( len-3,len).equals("\u00e1ch")|| //-ách
				    buffer.substring( len-3,len).equals("ata")||
				    buffer.substring( len-3,len).equals("aty")||
				    buffer.substring( len-3,len).equals("\u00fdch")||   //-ých
				    buffer.substring( len-3,len).equals("ama")||
				    buffer.substring( len-3,len).equals("ami")||
				    buffer.substring( len-3,len).equals("ov\u00e9")||   //-ové
				    buffer.substring( len-3,len).equals("ovi")||
				    buffer.substring( len-3,len).equals("\u00fdmi")){  //-ými
				
		                    buffer.delete( len- 3 , len);
		                    return;
				}
		}  
		if( len> 4){
				if(buffer.substring( len-2,len).equals("em")){
			
			         buffer.delete( len- 1 , len);
			         palatalise(buffer);
			         return;
			         
			      }
		                if( buffer.substring( len-2,len).equals("es")|| 
				    buffer.substring( len-2,len).equals("\u00e9m")||    //-ém
				    buffer.substring( len-2,len).equals("\u00edm")){   //-ím
			
			            buffer.delete( len- 2 , len);
			            palatalise(buffer);
			            return;
			        }
		                if( buffer.substring( len-2,len).equals("\u016fm")){  //-ům
			
			            buffer.delete( len- 2 , len);
			            return;
			        }
		                if( buffer.substring( len-2,len).equals("at")|| 
				    buffer.substring( len-2,len).equals("\u00e1m")||    //-ám
				    buffer.substring( len-2,len).equals("os")||
				    buffer.substring( len-2,len).equals("us")||   
				    buffer.substring( len-2,len).equals("\u00fdm")||     //-ým
				    buffer.substring( len-2,len).equals("mi")||   
				    buffer.substring( len-2,len).equals("ou")){
				
				    buffer.delete( len- 2 , len);
				    return;
				}
		}//len>4
		if( len> 3){
			 if( buffer.substring( len-1,len).equals("e")||
			    buffer.substring( len-1,len).equals("i")){
			
			     palatalise(buffer);
			     return;
			}
		            if( buffer.substring( len-1,len).equals("\u00ed")||  //-í
				    buffer.substring( len-1,len).equals("\u011b")){      //-ě
				
				    palatalise(buffer);
				    return;
				}
		            if( buffer.substring( len-1,len).equals("u")||
				    buffer.substring( len-1,len).equals("y")||
				    buffer.substring( len-1,len).equals("\u016f")){  //-ů
					
				    buffer.delete( len- 1 , len);
				    return;
				           
				}
		          if( buffer.substring( len-1,len).equals("a")||
		        	  buffer.substring( len-1,len).equals("o")||
				    buffer.substring( len-1,len).equals("\u00e1")||  // -á
				    buffer.substring( len-1,len).equals("\u00e9")||  //-é
				    buffer.substring( len-1,len).equals("\u00fd")){   //-ý
				
				    buffer.delete( len- 1 , len);
				    return;
				}
		}//len>3
	}
	

    private static void usage()
    {
        System.err.println("Usage: TestApp <algorithm> [<input file>] [-o <output file>]");
    }

    public static void main(String [] args) throws Throwable {
        if (args.length < 1) {
            usage();
            return;
        }

        CzechStemmerLight stemmer = new CzechStemmerLight();

	int arg = 1;

	InputStream instream;
	if (args.length > arg && !args[arg].equals("-o")) {
	    instream = new FileInputStream(args[arg++]);
	} else {
	    instream = System.in;
	}

        OutputStream outstream;
	if (args.length > arg) {
            if (args.length != arg + 2 || !args[arg].equals("-o")) {
                usage();
                return;
            }
	    outstream = new FileOutputStream(args[arg + 1]);
	} else {
	    outstream = System.out;
	}

	Reader reader = new InputStreamReader(instream, StandardCharsets.UTF_8);
	reader = new BufferedReader(reader);

	Writer output = new OutputStreamWriter(outstream, StandardCharsets.UTF_8);
	output = new BufferedWriter(output);

	StringBuffer input = new StringBuffer();
	int character;
	while ((character = reader.read()) != -1) {
	    char ch = (char) character;
	    if (Character.isWhitespace(ch)) {
		String result = stemmer.stem(input.toString());
		output.write(result);
		output.write('\n');
		input.delete(0, input.length());
	    } else {
		input.append(ch < 127 ? Character.toLowerCase(ch) : ch);
	    }
	}
	output.flush();
    }
	
}//CzechStemmer_1
