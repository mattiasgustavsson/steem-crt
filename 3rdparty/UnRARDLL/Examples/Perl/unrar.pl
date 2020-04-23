#Author  : Nikos Vaggalis
#Website : http://perlingresprogramming.blogspot.com/
#Email   : nikos.vaggalis@gmail.com
#Version Date: 16-11-2008
#To do for next version : better error description instead of just 'dying' plus more informative messages
#			  add support for using script on files not in script's directory  
#	  	          comment code			
#Wrapper for unrar.dll in Perl.Interacts with dll using the Win32::API module
#For feedback/comments/suggestions please use above email address

use Win32::API;
use Cwd;

sub declare_win32_functions {
    $RAROpenArchiveEx=new Win32::API('unrar.dll','RAROpenArchiveEx','P','N');
    $RARCloseArchive=new Win32::API('unrar.dll','RARCloseArchive','N','N');
    $RAROpenArchive=new Win32::API('unrar.dll','RAROpenArchive','P','N');
    $RARReadHeader=new Win32::API('unrar.dll','RARReadHeader','NP','N');
    $RARReadHeaderEx=new Win32::API('unrar.dll','RARReadHeaderEx','NP','N');
    $RARProcessFile=new Win32::API('unrar.dll','RARProcessFile','NNPP','N');
    $RARSetPassword=new Win32::API('unrar.dll','RARSetPassword','NP','N');
}
     
     
sub extract_headers {
    my $file=@_[0];
    my $CmtBuf = pack('x16384');

    my $RAROpenArchiveDataEx=pack('ppLLPLLLLx32',$file,undef,2,0,$CmtBuf,16384,0,0,0);
    my $RAROpenArchiveData=pack('pLLpLLL',$file,2,0,undef,0,0,0);
    my $RARHeaderData=pack('x260x260LLLLLLLLLLpLL',0,0,0,0,0,0,0,0,0,0,0,undef,0,0);
    
    my $handle = $RAROpenArchiveEx->Call($RAROpenArchiveDataEx)||die "RAROpenArchiveEx failed";
          
    my ($arcname,undef,undef,undef,$CmtBuf1,undef,$CmtSize,$CmtState,$flagsEX)=unpack('ppLLP16384LLLLL',$RAROpenArchiveDataEx);
    
    
    !$RARCloseArchive->Call($handle)||die "RARCloseArchive failed";
  
    my $handle = $RAROpenArchive->Call($RAROpenArchiveData)||die "RAROpenArchive failed";
         
    $flagsEX  & 128 || !$RARReadHeader->Call($handle,$RARHeaderData) || die "RARCloseArchive failed";
    #!$RARReadHeader->Call($handle,$RARHeaderData)||die "RARReadHeader failed";
   
    
    my ($arcname,$filename,$flags,$packsize)=unpack('A260A260LL',$RARHeaderData);


    
   if ($CmtBuf1) {
	$CmtBuf1=unpack('A'.$CmtSize,$CmtBuf1);
   }
   
    
  printf("\nArchive %s\n",$arcname);
  printf("\nVolume:\t\t%s",($flagsEX & 1) ? "yes":"no");
  printf("\nComment:\t%s",($flagsEX  & 2) ? "yes":"no");
  printf("\nLocked:\t\t%s",($flagsEX  & 4) ? "yes":"no");
  printf("\nSolid:\t\t%s",($flagsEX  & 8) ? "yes":"no");
  printf("\nNew naming:\t%s",($flagsEX  & 16) ? "yes":"no");
  printf("\nAuthenticity:\t%s",($flagsEX  & 32) ? "yes":"no");
  printf("\nRecovery:\t%s",($flagsEX  & 64) ? "yes":"no");
  printf("\nEncr.headers:\t%s",($flagsEX  & 128) ? "yes":"no");
  printf("\nFirst volume:\t%s",($flagsEX  & 256) ? "yes":"no or older than 3.0");
  printf("\n\nComments :\n%s",$CmtBuf1) if ($CmtBuf1);
  print "\n\n";
  
 !$RARCloseArchive->Call($handle);
 return ($flagsEX  & 128,$flags  & 4);
 
}

sub list_files_in_archive  {
my $file =@_[0];
my ($blockencrypted,$locked) = extract_headers($file);

my $password;

my $RAROpenArchiveDataEx_for_extracting=pack('ppLLpLLLLx32',$file,undef,2,0,undef,0,0,0,0);
my $handle = $RAROpenArchiveEx->Call($RAROpenArchiveDataEx_for_extracting)||die "RAROpenArchiveEx failed"; 
my $RARHeaderData=pack('x260x260LLLLLLLLLLpLL',0,0,0,0,0,0,0,0,0,0,0,undef,0,0,0); 

if ($blockencrypted){
    print ("Encrypted headers,enter password: "),chomp ($password=<STDIN>);
    
    if ($password) {
        $RARSetPassword->Call($handle,$password);
    }
    else {
	die "\nshould had entered password!!exiting....\n";
	}
}

    while (($RARReadHeader->Call($handle,$RARHeaderData))==0) {
       				    my $processresult=$RARProcessFile->Call($handle,0,undef,undef);
        			    if ($processresult!=0) {
           				last;
				    }  
				    else {
				       my  @files=unpack('A260A260LLLLLLLLLLpLL',$RARHeaderData);
                                        print "File\t\t\t\t\tSize\n";
                                        print "-------------------------------------------\n";
                                        print "$files[0]\\$files[1]\t\t$files[4]\n\n";
                                        }
				     
   }

 
  !$RARCloseArchive->Call($handle)||die "$RARCloseArchive failed";
}

sub process_file {
my $file=@_[0];
my ($blockencrypted,$locked) = extract_headers($file);

my $errorstatus;
my $password;

my $RAROpenArchiveDataEx_for_extracting=pack('ppLLpLLLLx32',$file,undef,1,0,undef,0,0,0,0);
my $RARHeaderData=pack('x260x260LLLLLLLLLLpLL',0,0,0,0,0,0,0,0,0,0,0,undef,0,0);
 
my $handle = $RAROpenArchiveEx->Call($RAROpenArchiveDataEx_for_extracting)||die "RAROpenArchiveEx failed"; 

if ($blockencrypted || $locked){
    print ("Enter password: "),chomp ($password=<STDIN>);
    
    if ($password) {
        $RARSetPassword->Call($handle,$password);
    }
    else {
	die "\nshould had entered password!!exiting....\n";
	}
}
       

    while (($RARReadHeader->Call($handle,$RARHeaderData))==0) {
       				     my $processresult=$RARProcessFile->Call($handle,2,undef,undef);
        			    if ($processresult!=0) {
                                        $errorstatus=$processresult;
           				last;
				    }  		
				     
   }

print "Errorstatus : $errorstatus " if $errorstatus;
!$RARCloseArchive->Call($handle)||die "RRARCloseArchive failed"; 
}

declare_win32_functions();

my $file;
if (!defined @ARGV) {
    print "Usage : perl unrar.pl [option] filename\n";
    print "where [option] is either X or L. (X=extract,L=List file contents)\n";
    print "\nThe script and the file to be processed should be in the same directory\n";
    print "\nExample usage: \nperl unrar.pl L myfile.rar\n";
    }
elsif (!defined $ARGV[0] || $ARGV[0] ne "L" && $ARGV[0] ne "X") {  
    print "Enter mode L or X\n";
    }
elsif (!defined $ARGV[1]) {   
    print ("No filename!") && exit;
    }
else {
    $file=$ARGV[1];
    if ($ARGV[0] eq "L") {
                          list_files_in_archive($file);
                          }
    elsif ($ARGV[0] eq "X") {
                          process_file($file);
                          }
}    

