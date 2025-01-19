/*
  Javascript
*/
const EARTH_RADIUS = 6371000; // 地球の半径（メートル）
const base_url = "https://map.umineco.company/"

let img_width = 10;  // メートル
let img_height = 8;  // メートル

let img_path = ''
let map = null // Global object
let idx_array = []

// implicit setup
const selectdate = document.getElementById("selectdate")
const buildButton = document.getElementById("buildbutton")

// インデックスファイルを選ぶ。ファイル名からフォルダー名が確定する
selectdate.addEventListener("change", function(event){
    var index = event.selectedIndex
    const idx_file = event.target.value
    
    if (idx_file != ''){
        const filepath = idx_file.split("/")
        img_path = filepath[0]
        getIndex(idx_file) // read index, but this is async.
        buildButton.disabled = false
    }
    
})

//　マップを描いて写真をはる
buildButton.addEventListener("click", function(event){
    buildButton.disabled = true
    // 写真の一辺を何メートルとするかを指定。具体的には緯度経度の倍率となる。
    const iwidth = document.getElementById("imgWidth").value
    img_width = parseFloat(iwidth)
    const iheight = document.getElementById("imgHeight").value
    img_height = parseFloat(iheight)
    if (iwidth == "" || iheight =="") {
        alert("幅か高さがセットされていません。処理を中止します")
        exit()
    } else {
        console.log("Start map process.")
    }
    
    const [timestamp, filename, lat, lon, alt] = idx_array[0]
    map_init(parseFloat(lat), parseFloat(lon))

    pastes() // 写真の貼り付け
    buildButton.disabled = false
})

// インデックスファイルを読み取り、レコード、フィールドに分割する
function getIndex(idx_file){
    const indexfile = base_url + idx_file
    console.log(indexfile)
    const xhr = new XMLHttpRequest()
    xhr.open("GET", indexfile, false)
    xhr.send(null)
    if (xhr.status == 200){
        const csvData = xhr.responseText
        const rows = csvData.split("\n")
        idx_array =  rows.map(row => row.split(","))
    } else {
        alert(indexfile + ": が読み取れません")
    }
}

// インデックスファイルを読み取り、overlayに貼り付ける
function pastes(){

  for (const record of idx_array){
    [timestamp, filename, lat, lon, alt, direction] = record

    // check existance of file.
    img_url = base_url + img_path + '/' + filename
    // xhr = new XMLHttpRequest()
    // xhr.open("GET", img_url, false)
    // xhr.send(null)

    // if (xhr.status == 200){
        console.log(img_url+" processing...")
        paste1(img_url, parseFloat(lat), parseFloat(lon), parseFloat(direction) )
    // }
    // break; // one record only
  }
  alert("処理は終了しました。描画には時間がかかります。")
}

// 一枚貼り付ける
function paste1(img_url, centerLat, centerLon, direction){
    bounds = calculateRotatedImageBounds(centerLat, centerLon, direction, img_width, img_height);
    const rotatedOverlay = L.imageOverlay.rotated(
        img_url, 
        bounds.topLeft, 
        bounds.topRight, 
        bounds.bottomLeft,
        {
            opacity: 0.8,
            interactive: false
         }
    ).addTo(map);
}

// 以下、緯度経度のローテーション
function calculateRotatedImageBounds(centerLat, centerLon, direction, width, height) {
  // 方向をラジアンに変換
  const angle = (direction * Math.PI) / 180;

  // 画像の半分のサイズを計算
  const halfWidth = width / 2;
  const halfHeight = height / 2;

  // 回転行列を適用する関数
  function rotatePoint(x, y) {
    const rotatedX = x * Math.cos(angle) - y * Math.sin(angle);
    const rotatedY = x * Math.sin(angle) + y * Math.cos(angle);
    return [rotatedX, rotatedY];
  }

  // 3つの角の座標を計算
  const topLeft = rotatePoint(-halfWidth, halfHeight);
  const topRight = rotatePoint(halfWidth, halfHeight);
  const bottomLeft = rotatePoint(-halfWidth, -halfHeight);

  // 緯度経度に変換（簡易的な平面近似）
  const latPerMeter = 1 / 111111;  // 1度あたりのメートル数の逆数（概算）
  const lonPerMeter = 1 / (111111 * Math.cos(centerLat * Math.PI / 180));

  function toLatLng(point) {
      const lat = centerLat + point[1] * latPerMeter;
      const lon = centerLon + point[0] * lonPerMeter;
      return L.latLng(lat, lon);
  }

    // 3点の緯度経度を返す
  return  {
      topLeft: toLatLng(topLeft),
      topRight: toLatLng(topRight),
      bottomLeft: toLatLng(bottomLeft)
  };
}

// mapの描画
function map_init(lat, lon){
    // lat = 38.3995807  // temporary
    // lon = 141.4103738 // temporary
    map = L.map('maparea',{zoomControl: false})
    //地図の中心とズームレベルを指定
    map.setView([lat, lon], 18)
    //表示するタイルレイヤのURLとAttributionコントロールの記述を設定して、地図に追加する
    var gsi = L.tileLayer('https://cyberjapandata.gsi.go.jp/xyz/std/{z}/{x}/{y}.png', 
        {zoomSnap: 0.1, zoom: 18, maxNativeZoom:18,maxZoom:25,attribution: "<a href='https://maps.gsi.go.jp/development/ichiran.html' target='_blank'>地理院標準タイル</a>",})
    var gsiphoto = L.tileLayer('https://cyberjapandata.gsi.go.jp/xyz/seamlessphoto/{z}/{x}/{y}.jpg',
    {zoomSnap: 0.1, zoom: 18, maxNativeZoom:18,maxZoom:25,attribution: "<a href='http://portal.cyberjapan.jp/help/termsofuse.html' target='_blank'>地理院写真タイル</a>"});
    var gsipale = L.tileLayer('http://cyberjapandata.gsi.go.jp/xyz/pale/{z}/{x}/{y}.png',
    {zoomSnap: 0.1, zoom: 18, maxNativeZoom:18,maxZoom:25,attribution: "<a href='http://portal.cyberjapan.jp/help/termsofuse.html' target='_blank'>地理院淡色タイル</a>"});
    //オープンストリートマップのタイル
    var osm = L.tileLayer('http://tile.openstreetmap.jp/{z}/{x}/{y}.png',
    {zoomSnap: 0.1, zoom: 18, maxNativeZoom:18,maxZoom:25,attribution: "<a href='http://osm.org/copyright' target='_blank'>OpenStreetMap</a>" });
  
    //baseMapsオブジェクトのプロパティに3つのタイルを設定
    var baseMaps = {
        '地理院地図' : gsi,
        '地理院写真' : gsiphoto,
        '地理院淡色' : gsipale,
        'OSM' : osm,
    };
    // layers コントロールにbaseMapsおｂジェクトを設定して地図に追加
    // コントロール内にプロパティがでる
    L.control.layers(baseMaps).addTo(map);
    // とりあえずデフォルト
    gsi.addTo(map);
    //スケールコントロールを最大幅200px,右下、単位ｍで地図に追加
    L.control.scale({ maxWidth: 200, position: 'bottomright', imperial: false }).addTo(map);
    //ズームコントロールを左下に
    L.control.zoom({ position: 'bottomleft'}).addTo(map);

}

  
