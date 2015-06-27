<?php

require_once('../src/Wechat.php');
require_once('../config.php');
/**
 * 微信公众平台文本类
 */
class WechatText
{
    public function __construct($wechat)
    {
        $this->m_wechat = $wechat;
    }

    public function run()
    {
        $this->onText();
    }

    protected function responseNews($title, $description, $image_url, $redirect_url)
    {
        $item = new NewsResponseItem($title, $description, $image_url, $redirect_url);
        exit(new NewsResponse($this->m_wechat->getRequest('fromusername'), $this->m_wechat->getRequest('tousername'), array($item), 0));
    }

    protected function responseText($content, $funcFlag = 0)
    {
        exit(new TextResponse($this->m_wechat->getRequest('fromusername'), $this->m_wechat->getRequest('tousername'), $content, $funcFlag));
    }

    /**
     * application | CREATE TABLE `application` (
     * `id` int(11) NOT NULL AUTO_INCREMENT,
     * `session` varchar(8) NOT NULL,
     * `patient_name` varchar(32) NOT NULL,
     * `tel` varchar(16) NOT NULL,
     * `age` smallint(6) NOT NULL,
     * `disease` int(11) NOT NULL,
     * `symptom` int(11) NOT NULL,
     * PRIMARY KEY (`id`)
     * ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
     * @return bool
     */
    protected function onText()
    {
        $connection = connect_db(HOST, PORT, USER, PASSWD, DBNAME);
        $mysqli = new mysqli(HOST, USER, PASSWD, DBNAME);

        if (NULL == $connection) {
            $this->responseText('连接数据库失败');
        } else {

            $str = explode(" ", $this->m_wechat->getRequest('content'));

            if (count($str) < 6 || $str[0] != '报名') {
                $this->responseText("您的格式不对，请重新输入。\n例如：报名 1期 陈小明 13888769890 20 蛀牙");
            }

            $session_number = intval($str[1]);
            if ($session_number < 1) {
                $this->responseText('请输入正确的期数');
            }

            $get_session_sql = "select * from activity where session_no = {$session_number} limit 1";
            $session_result = $mysqli->query($get_session_sql);
            $session = $session_result->fetch_array();

            if (empty($session)) {
                $this->responseText("您输入了{$session_number}，但它并不是有效的期数");
            }

            $count_result = $mysqli->query("select count(*) as count from application where session_no = {$session_number};");
            $count_result = $count_result->fetch_array();

            #mysql_query("SET NAMES 'utf8'");
            $sql = "INSERT INTO `application`(`session_no`, `patient_name`, `tel`, `age`, `disease`, `symptom`, `created_at`, `updated_at`)
                    VALUES ('%s','%s','%s','%s','%s', '%s', now(), now())";
            $sql = sprintf($sql, $session_number, $str[2], $str[3], intval($str[4]), $str[5], isset($str[6]) ?: '');
            mysql_query($sql, $connection);


            if ($count_result['count'] > $session['capacity']) {
                $this->responseText('本期人数容量为' . $session['capacity'] . '，报名人数已满，敬请留意下期专家咨询讲座，感谢您的参与！');
            } else {
                $prefix = 'http://gz.pear.hk/test';
                $this->responseNews($session['title'], '感谢您的参与，请长按二维码加入微信咨询群', $prefix . $session['pic_url'], $prefix . $session['redirect_url']);
            }
        }

        $this->responseText('收到了文字消息：' . $this->m_wechat->getRequest('content'));
    }

    private $m_wechat;
}

function connect_db($host, $port, $user, $passwd, $dbname)
{
    $c = mysql_connect($host . ":" . $port, $user, $passwd, true);
    if ($c) {
        if (mysql_select_db($dbname))
            return $c;
    }
    return NULL;
}

