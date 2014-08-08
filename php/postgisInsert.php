
<?php
  include 'RESTpasswords.php';
  /*echo strval($_POST["data"]);*/
  //if(!$dbh){
  //  die("Error in connection: " . pg_last_error());
  //}

  if($_POST['hash']=hash('sha256','$sensorpass')){  
    if($_POST['submit']){
      $dbh = pg_connect("dbname=$dbname user=$dbuser password=$dbpass");
      if(!$dbh){
        die("Error in connection: " . pg_last_error());
      }

      $data = pg_escape_string($_POST['data']);
      $locid = pg_escape_string($_POST['loc_id']);

      $sql = "INSERT INTO sensordata(loc_id,temp) VALUES('$locid', '$data')";
      $result = pg_query($dbh, $sql);

      if(!result){
        die("Error in SQL query: " . pg_last_error());
      }

      echo "Data successfully inserted!";
      pg_free_result($result);
      pg_close($dbh);

    }
  }
?>
