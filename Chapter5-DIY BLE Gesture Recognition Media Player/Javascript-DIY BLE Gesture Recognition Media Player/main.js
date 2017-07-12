const serviceUuid = "19b10000-e8f2-537e-4f6c-d104768a1216"; // lowercase hex characters e.g. '00001234-0000-1000-8000-00805f9b34fb'
const name = 'CuriePME';

var bluetoothDevice;
var characteristiPattern;

var value;
var letter = '';
var index;
var song;
var texts = 'Waiting...';

var backgroundColors = ['#1DFFAD', '#0FE8E2'];

function preload() {
  song = loadSound('./song.mov');
}

function setup() {
  createCanvas(windowWidth, windowHeight);
  textSize(180);
  textAlign(CENTER);
  noStroke();
  fill('#FF5797');
  song.setVolume(0.5);
}

function draw() {
  if (letter !== '') {
    changeColors();
  }
  text(texts, width / 2, height / 2);
}

function changeColors() {
  if (letter === 'A') {
    index = 0;
    texts = 'play';
    if (!song.isPlaying()) song.play();
  } else if (letter === 'B') {
    index = 1;
    texts = 'pause';
    if (song.isPlaying()) song.pause();
  }
  background(backgroundColors[index]);
}
