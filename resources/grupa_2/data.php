<?php
//curl request returns json output via json_decode php function
function curl($url){
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $url);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
	curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.2) Gecko/20090729 Firefox/3.5.2 GTB5');
	$result = curl_exec($ch);
	curl_close($ch);
	return $result;
}

//parse the json output
function getResults($json){

	$results = array();

	$json_array = json_decode($json, true);

	foreach($json_array['query']['pages'] as $page){
		if(count($page['images']) > 0){
		    foreach($page['images'] as $image){

		    	$title = str_replace(" ", "_", $image["title"]);
		    	$imageinfourl = "http://en.wikipedia.org/w/api.php?action=query&titles=".$title."&prop=imageinfo&iiprop=url&format=json";
		    	$imageinfo = curl($imageinfourl);
		    	$iamge_array = json_decode($imageinfo, true);
		    	$image_pages = $iamge_array["query"]["pages"];

				foreach($image_pages as $a){
					$results[] = $a["imageinfo"][0]["url"];
					
				}
			}
		}
	}

	return $results;

}


$search = $_GET["q"];

if (empty($search) || glob ($search.'*')) {
    //term param not passed in url
    echo 'empty search or search already done';
    exit;
} else {
    //create url to use in curl call
    $term = str_replace(" ", "_", $search);
    $url = "http://en.wikipedia.org/w/api.php?action=query&titles=".$term."&prop=images&format=json&imlimit=500";

    $json = curl($url);
    $results = getResults($json);
		

	$c=0;
	
	foreach($results as $a){
		
		
	$pattern = '/.svg/i';
	
	if(!preg_match($pattern, $a, $matches))
	{
	    
	
	$c++;
	
	$plant_part = 'Entire';
	

if (preg_match('/flower/i', $a)
|| preg_match('/flowers/i', $a)
|| preg_match('/flora/i', $a)
|| preg_match('/floret/i', $a)
|| preg_match('/inflorescence/i', $a)
|| preg_match('/bloom/i', $a)
|| preg_match('/blossom/i', $a))
$plant_part = "Flower";

else if (preg_match('/bark/i', $a)
|| preg_match('/wood/i', $a)
|| preg_match('/trunk/i', $a))
$plant_part = "Stem";

else if (preg_match('/leaf/i', $a)
|| preg_match('/leaves/i', $a)
|| preg_match('/folie/i', $a)
|| preg_match('/foliage/i', $a))
$plant_part = "Leaf";

else if (preg_match('/fruit/i', $a)
|| preg_match('/fruits/i', $a)
|| preg_match('/seed/i', $a)
|| preg_match('/seeds/i', $a)
|| preg_match('/fructus/i', $a))
$plant_part  = "Fruit";

else if (preg_match('/branch/i', $a)
|| preg_match('/branches/i', $a) )
$plant_part = "Branch";

else if ((preg_match('/scan/i', $a) && preg_match('/leaf/i', $a)) || (preg_match('/scans/i', $a) && preg_match('/leaves/i', $a)) )
$plant_part = "LeafScan";

$extension = pathinfo($a, PATHINFO_EXTENSION);

$img = $search.$c.'.'.$extension;

//you must check folder permissions in order for files to be saved

file_put_contents($img, file_get_contents($a));

$fp=fopen($search.$c.'.xml',"w");

fwrite($fp,'<?xml version="1.0" encoding="UTF-8"?>'.PHP_EOL);
fwrite($fp,'<Image>'.PHP_EOL);
fwrite($fp,'  <ObservationId>'.$search.'</ObservationId>'.PHP_EOL);
fwrite($fp,'  <FileName>'.$img.'</FileName>'.PHP_EOL);
fwrite($fp,'  <MediaId>'.$search.$c.'</MediaId>'.PHP_EOL);
fwrite($fp,'  <Vote>'.mt_rand(5, 10).'</Vote>'.PHP_EOL);
fwrite($fp,'  <Content>'.$plant_part.'</Content>'.PHP_EOL);
fwrite($fp,'  <ClassId>'.mt_rand(1,32000).'</ClassId>'.PHP_EOL);
fwrite($fp,'  <Family>'.$search.'</Family>'.PHP_EOL);
fwrite($fp,'  <Species>'.$search.'</Species>'.PHP_EOL);
fwrite($fp,'  <Genus>'.$search.'</Genus>'.PHP_EOL);
fwrite($fp,'  <Author>B6</Author>'.PHP_EOL);
fwrite($fp,'  <Date>'.date("d/m/y").'</Date>'.PHP_EOL);
fwrite($fp,'  <Location />'.PHP_EOL);
fwrite($fp,'  <Latitude />'.PHP_EOL);
fwrite($fp,'  <Longitude />'.PHP_EOL);
fwrite($fp,'  <YearInCLEF>PlantCLEF2015</YearInCLEF>'.PHP_EOL);
fwrite($fp,'  <IndividualPlantId2015 />'.PHP_EOL);
fwrite($fp,'  <ImageID2015 />'.PHP_EOL);
fwrite($fp,'  <LearnTag>Train</LearnTag>'.PHP_EOL);
fwrite($fp,'</Image>'.PHP_EOL);

fclose($fp);

	}
    }
}

?>
