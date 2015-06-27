<?php
require_once ('../src/Wechat.php');
define("HOST", "rdsn5he7i2sl8maufj3z6.mysql.rds.aliyuncs.com");
define("PORT", "3306");
define("USER", "roqihi4cch4o222z");
define("PASSWD", "107842");
define("DBNAME", "roqihi4cch4o222z");
define("TABLE", "driving");
/**
 * 微信公众平台文本类
 */
class WechatText{
	public function __construct($wechat) {
		$this->m_wechat = $wechat;
	}

	public function run()
	{
		$this->onText();  
	}

	protected function responseText($content, $funcFlag = 0) {
		exit(new TextResponse($this->m_wechat->getRequest('fromusername'), $this->m_wechat->getRequest('tousername'), $content, $funcFlag));
	}

	protected function onText() {
		$c = connect_db(HOST, PORT, USER, PASSWD, DBNAME);
		if (NULL == $c ){
			$this->responseText('连接数据库失败');
		} else {
			$str = explode (" ",$this->m_wechat->getRequest('content'),3);
			$ret = query_db($str,$c,$this->m_wechat);
			if(!$ret)
                {
                        mysql_close($c);
                        return false;
            }
			
			$result = array();
			$num = 0;
            while($row = mysql_fetch_assoc($ret))
            {
                $result[$num] = $row["question"].'   '.$row["answer1"].$row["answer2"].$row["answer3"].$row["answer4"].'   '.$row["answer5"];
				$num = $num + 1;
            }
            mysql_close($c);
             #  return $result;
			 if (count($result)> 1) {
				$this->responseText('找到了'.count($result).'个题目，给出一个结果:'."\n".$result[0]); 
			 } else if(count($result) == 0){ 
				$this->responseText('没有找到匹配的题目，换一个关键字试试？');
			 } else {
				$this->responseText($result[0]);
			 }
		}
		$this->responseText('收到了文字消息：' . $this->m_wechat->getRequest('content'));
	}
	private $m_wechat;
}

function connect_db($host, $port, $user, $passwd, $dbname)
{
	$c = mysql_connect($host.":".$port, $user, $passwd, true);
	if($c)
	{
		if(mysql_select_db($dbname))
			return $c;
	}
	return NULL;
}

function execute_query($query, $pdo)
{
	return mysql_query($query, $pdo);
}
function query_db($query, $pdo,$o)
{
	$str = "";
	if (count($query)==0) {
		return 0;
	} else if (count($query)==1) {
		$template = <<<TEMP
select * from %s where question like '%%%s%%'
TEMP;
	$str = sprintf($template,TABLE,$query[0]);
	} else if (count($query)==2) {
		$template = <<<TEMP
select * from %s where question like '%%%s%%%s%%'
TEMP;
	$str = sprintf($template,TABLE,$query[0],$query[1]);
	} else if (count($query)==3) {
		$template = <<<TEMP
select * from %s where question like '%%%s%%%s%%%s%%'
TEMP;
	$str = sprintf($template,TABLE,$query[0],$query[1],$query[2]);
	}
	$str;
	#exit(new TextResponse($o->getRequest('fromusername'), $o->getRequest('tousername'), $str,0));
	return mysql_query($str, $pdo);
}

?>
