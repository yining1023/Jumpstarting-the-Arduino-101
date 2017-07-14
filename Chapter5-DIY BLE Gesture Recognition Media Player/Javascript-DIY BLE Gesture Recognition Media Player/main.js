var song;
var displayText = 'Waiting...';

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
  if (pattern !== '') {
    patternMatched();
  }
  text(displayText, width / 2, height / 2);
}

function patternMatched() {
  // if pattern A is recognized, play the song
  // else pause it
  if (pattern === 'A') {
    displayText = 'play';
    background(backgroundColors[0]);
    if (!song.isPlaying()) {
      console.log('Playing Song...');
      song.play();
    }
  } else if (pattern === 'B') {
    displayText = 'pause';
    background(backgroundColors[1]);
    if (song.isPlaying()) {
      console.log('Pausing Song...');
      song.pause();
    }
  }
}
