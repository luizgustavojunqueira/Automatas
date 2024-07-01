const canvas = document.getElementById("gameCanvas");
const ctx = canvas.getContext("2d");

const ruleselector = document.getElementById("ruleselector");
const simulateButton = document.getElementById("simulate");
const cellSizeSelector = document.getElementById("cellSizeSelector");

let ruleDecimal = 0;
let cellSize = 20;

ruleselector.addEventListener("input", (e) => {
  ruleDecimal = parseInt(e.target.value);

  ruleDecimal > 255 ? 255 : ruleDecimal;
});

simulateButton.addEventListener("click", () => {
  simulate();
});

cellSizeSelector.addEventListener("input", (e) => {
  cellSize = parseInt(e.target.value);

  cellSize > 10 ? 10 : cellSize;
});

let currentState = [];
let cols = 0;
let row = 0;
let width = 0;
let height = 0;

function initializeState(size) {
  let state = new Array(size);

  for (let i = 0; i < size; i++) {
    state[i] = 0;
  }

  return state;
}

function drawState() {
  for (let i = 0; i < currentState.length; i++) {
    if (currentState[i] == 0) {
      ctx.fillStyle = "#FFF";
      ctx.fillRect(i * cellSize, row * cellSize, cellSize, cellSize);
    } else {
      ctx.fillStyle = "#000";
      ctx.fillRect(i * cellSize, row * cellSize, cellSize, cellSize);
    }
  }
}

function simulate() {
  ctx.clearRect(0, 0, width, height);
  let rule = ruleDecimal.toString(2);

  while (rule.length < 8) {
    rule = "0" + rule;
  }
  cols = width / cellSize;

  currentState = initializeState(cols);

  currentState[Math.floor(currentState.length / 2)] = 1;

  for (let i = 0; i < height / cellSize; i++) {
    row = i;
    drawState();
    currentState = calculateNewState(rule);
  }
}

function calculateNewState(rule) {
  let newState = initializeState(cols);

  newState[0] = currentState[0];
  newState[currentState.length - 1] = currentState[currentState.length - 1];

  for (let i = 1; i < currentState.length - 1; i++) {
    let current = currentState[i];
    let left = currentState[i - 1];
    let rigth = currentState[i + 1];

    let neighborhood = left.toString() + current.toString() + rigth.toString();

    switch (neighborhood) {
      case "000":
        newState[i] = rule[7];
        break;
      case "001":
        newState[i] = rule[6];
        break;
      case "010":
        newState[i] = rule[5];
        break;
      case "011":
        newState[i] = rule[4];
        break;
      case "100":
        newState[i] = rule[3];
        break;
      case "101":
        newState[i] = rule[2];
        break;
      case "110":
        newState[i] = rule[1];
        break;
      case "111":
        newState[i] = rule[0];
        break;
    }
  }

  return newState;
}

function setup() {
  resizeCanvasToDisplaySize();

  simulate();
}

setup();

function resizeCanvasToDisplaySize() {
  // look up the size the canvas is being displayed
  width = canvas.clientWidth;
  height = canvas.clientHeight;

  width = width - (width % cellSize);
  height = height - (height % cellSize);

  // If it's resolution does not match change it
  if (canvas.width !== width || canvas.height !== height) {
    canvas.width = width;
    canvas.height = height;
    return true;
  }

  return false;
}
