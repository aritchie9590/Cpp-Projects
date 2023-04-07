#!/bin/csh
set user = $1
set Expected = HelloFrom.aritchie3.YbZEBgjYaVwucgPIdrWzBmUQfUCZCkjoaaDAIDqp
set Actual = `./BreakRSA  16137419758697748421 5602749705103884343  1603097566384582710 14477098652127151979 5509146246724185369 1125729357419926785 15045311331593908621 337433295535627485 12411816998937694185 2857571383844514122 3047341403916544434 12395175534973750294`
echo "expected is $Expected" 
echo "actal    is $Actual"
if ( "$Expected" == "$Actual" ) then
echo "Grade for user $user is 100"
else
echo "Grade for user $user is 50"
endif
