let simulationRunning = false;
let gameState = initializeState(40, 40);

function drawGrid(canvas, ctx) {
  ctx.fillStyle = "black";
  ctx.clearRect(0, 0, canvas.clientWidth, canvas.clientHeight);
  ctx.fillRect(0, 0, canvas.clientWidth, canvas.clientHeight);

  ctx.beginPath();

  for (let i = 1; i < canvas.clientWidth / 20; i++) {
    ctx.moveTo(i * 20, 0);
    ctx.lineTo(i * 20, 800);
  }
  for (let i = 1; i < canvas.clientHeight / 20; i++) {
    ctx.moveTo(0, i * 20);
    ctx.lineTo(800, i * 20);
  }

  ctx.strokeStyle = "red";
  ctx.stroke();
}

function getMousePos(canvas, e) {
  let rect = canvas.getBoundingClientRect();

  let x = Math.floor(e.clientX - rect.left);
  let y = Math.floor(e.clientY - rect.top);

  x = x - (x % 20);
  y = y - (y % 20);

  return [x, y];
}

function drawCell(canvas, ctx, e) {
  if (!simulationRunning) {
    let [x, y] = getMousePos(canvas, e);

    gameState[y / 20][x / 20] = 1;
    console.log(gameState);
    drawGameState(ctx, gameState, canvas);
  }
}

function initializeState(width, height) {
  let state = [];

  for (let i = 0; i < height; i++) {
    state.push([]);
    for (let j = 0; j < width; j++) {
      state[i].push(0);
    }
  }

  return state;
}

function drawGameState(ctx, gameState, canvas) {
  drawGrid(canvas, ctx);
  ctx.fillStyle = "yellow";

  for (let i = 0; i < gameState.length; i++) {
    for (let j = 0; j < gameState.length; j++) {
      if (gameState[i][j] == 1) {
        ctx.fillRect(j * 20, i * 20, 20, 20);
      }
    }
  }
}

function getNeighbors(x, y) {
  let numNeighbors;

  if (x == 0 && y == 0) {
    numNeighbors =
      gameState[x + 1][y] + gameState[x + 1][y + 1] + gameState[x][y + 1];
  } else if (x == 39 && y == 39) {
    numNeighbors =
      gameState[x - 1][y] + gameState[x - 1][y - 1] + gameState[x][y - 1];
  } else if (x == 39 && y == 0) {
    numNeighbors =
      gameState[x - 1][y] + gameState[x - 1][y + 1] + gameState[x][y + 1];
  } else if (x == 0 && y == 39) {
    numNeighbors =
      gameState[x + 1][y] + gameState[x + 1][y - 1] + gameState[x][y - 1];
  } else if (x > 0 && y == 0) {
    numNeighbors =
      gameState[x - 1][y] +
      gameState[x - 1][y + 1] +
      gameState[x][y + 1] +
      gameState[x + 1][y] +
      gameState[x + 1][y + 1];
  } else if (x > 0 && y == 39) {
    numNeighbors =
      gameState[x - 1][y] +
      gameState[x - 1][y - 1] +
      gameState[x][y - 1] +
      gameState[x + 1][y] +
      gameState[x + 1][y - 1];
  } else if (x == 0 && y > 0) {
    numNeighbors =
      gameState[x][y - 1] +
      gameState[x][y + 1] +
      gameState[x + 1][y] +
      gameState[x + 1][y - 1] +
      gameState[x + 1][y + 1];
  } else if (x == 39 && y > 0) {
    numNeighbors =
      gameState[x - 1][y] +
      gameState[x - 1][y - 1] +
      gameState[x - 1][y + 1] +
      gameState[x][y - 1] +
      gameState[x][y + 1];
  } else if (x > 0 && y > 0) {
    numNeighbors =
      gameState[x - 1][y] +
      gameState[x - 1][y - 1] +
      gameState[x - 1][y + 1] +
      gameState[x][y - 1] +
      gameState[x][y + 1] +
      gameState[x + 1][y] +
      gameState[x + 1][y - 1] +
      gameState[x + 1][y + 1];
  }

  return numNeighbors;
}

function simulate() {
  let newGameState = gameState;
  for (let i = 0; i < gameState.length; i++) {
    for (let j = 0; j < gameState.length; j++) {
      let nN = getNeighbors(i, j);
      if (gameState[i][j] == 1) {
        if (nN > 0) console.log(i, j, nN);
        if (nN == 0 || nN == 1) {
          newGameState[i][j] = 0;
        } else if (nN == 2 || nN == 3) {
          newGameState[i][j] = 1;
        } else {
          newGameState[i][j] = 0;
        }
      } else {
        if (nN > 0) console.log("morto", i, j, nN);
        if (nN == 3) {
          newGameState[i][j] = 1;
        }
      }
    }
  }

  console.log("iteration");
}

function startSimulation(ctx, canvas) {
  simulationRunning = true;
  setInterval(() => {
    simulate();
    drawGameState(ctx, gameState, canvas);
  }, 1000);
}

function startGame() {
  // Get the canvas element
  const canvas = document.getElementById("gameCanvas");

  const ctx = canvas.getContext("2d");

  // Draw game grid
  drawGrid(canvas, ctx);

  // Add mouse click event listener
  canvas.addEventListener("mousedown", (e) => drawCell(canvas, ctx, e));

  const startButton = document.getElementById("start");
  startButton.addEventListener("click", () => startSimulation(ctx, canvas));

  drawGameState(ctx, gameState, canvas);
}

startGame();
