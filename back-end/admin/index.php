<?php

require_once('../config.php');

if ($_POST) {

    $mysqli = new mysqli(HOST, USER, PASSWD, DBNAME);
    $title = $_POST['title'];
    $session_number = intval($_POST['session_number']);
    $started_at = $_POST['started_at'];
    $capacity = intval($_POST['capacity']);
    $comment = $_POST['comment'];

    $cover = $_FILES['cover'];
    $qrcode = $_FILES['qrcode'];

    $cover_path = '/data/' . sha1(md5(rand(3, 100))) . '.jpg';
    $qrcode_path = '/data/' . sha1(md5(rand(3, 100))) . '.jpg';
    $cover_upload_path = ROOT . $cover_path;
    $qrcode_upload_path = ROOT . $qrcode_path;

    if (move_uploaded_file($_FILES['cover']['tmp_name'], $cover_upload_path)) {

    }

    if (move_uploaded_file($_FILES['qrcode']['tmp_name'], $qrcode_upload_path)) {

    }

    $insert_sql = "INSERT INTO `activity`(`session_no`, `capacity`, `title`, `pic_url`, `redirect_url`, `start_time`, `remark`) VALUES (%s, %s, '%s', '%s', '%s', '%s', '%s')";
    $insert_sql = sprintf($insert_sql, $session_number, $capacity, $title, $cover_path, $qrcode_path, $started_at, $comment);

    if ($mysqli->query($insert_sql)) {
        $message = '创建成功';
    } else {
        $message = '创建失败';
    }
}

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>管理</title>
    <link href="assets/bootstrap.min.css" rel="stylesheet">
    <link href="assets/bootstrap-datepicker.min.css" rel="stylesheet">
    <script src="assets/jquery-1.11.3.min.js"></script>
    <script src="assets/bootstrap-datepicker.min.js"></script>
    <script>
        $(function () {
            $('.datepicker').datepicker({
                format: "yyyy-mm-dd",
                keyboardNavigation: false,
                calendarWeeks: true,
                todayHighlight: true
            });
        })
    </script>
</head>

<body>

<div class="container">

    <?php if ($message) { ?>
        <div class="alert alert-success" role="alert"><?php echo $message; ?></div>
    <?php } ?>


    <form class="form-horizontal" method="post" enctype="multipart/form-data">
        <legend>添加讲座</legend>
        <div class="form-group">
            <label for="title" class="col-sm-2 control-label">讲座标题</label>

            <div class="col-sm-10">
                <input type="text" class="form-control" id="title" name="title" placeholder="请输入讲座的标题">
            </div>
        </div>
        <div class="form-group">
            <label for="session_number" class="col-sm-2 control-label">讲座期数</label>

            <div class="col-sm-10">
                <input type="number" class="form-control" id="session_number"
                       value="<?php echo strip_tags($session_number); ?>" name="session_number"
                       placeholder="请输入讲座的期数（纯数字）">
            </div>
        </div>
        <div class="form-group">
            <label for="started_at" class="col-sm-2 control-label">讲座开始时间</label>

            <div class="col-sm-10">
                <input type="text" class="form-control datepicker" id="started_at" value="" name="started_at">
            </div>
        </div>
        <div class="form-group">
            <label for="cover" class="col-sm-2 control-label">讲座封面</label>

            <div class="col-sm-10">
                <input type="file" class="form-control" id="cover" value="" name="cover">
            </div>
        </div>
        <div class="form-group">
            <label for="qrcode" class="col-sm-2 control-label">讲座微信群二维码</label>

            <div class="col-sm-10">
                <input type="file" class="form-control" id="qrcode" value="" name="qrcode">
            </div>
        </div>
        <div class="form-group">
            <label for="capacity" class="col-sm-2 control-label">讲座人数上限</label>

            <div class="col-sm-10">
                <input type="number" class="form-control" id="capacity" value="" name="capacity"
                       placeholder="请输入讲座的人数上限（纯数字）">
            </div>
        </div>
        <div class="form-group">
            <label for="comment" class="col-sm-2 control-label">讲座备注</label>

            <div class="col-sm-10">
                <input type="text" class="form-control" id="comment" value="" name="comment">
            </div>
        </div>

        <div class="form-group">
            <div class="col-sm-offset-2 col-sm-10">
                <input type="submit" value="提交" class="btn btn-primary"/>
            </div>
        </div>
    </form>

</div>
</body>
</html>
