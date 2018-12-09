<?php 
include('inc/myconnect.php');

$seri = $_GET['seri'];

if($seri == 9) {
	mysqli_query($dbc, "INSERT INTO test(cnt) VALUES (1)");
}


else {
	echo "Can't identify the <b>idD</b>";
}

mysqli_close($dbc);		  
?>
