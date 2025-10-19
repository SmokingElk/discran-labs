const f = (n, m = 1) => {
    let cnt = 0;
    for (let i = 1; i < n; i++) {
        if (String(i) < String(n) && i % m == 0) {
            cnt++;
        }
    }
    return cnt;
}

let delsInRange = (from, to, m) => {
  let lower = (from / m | 0) * m;
  if (from % m != 0) lower += m;
  let upper = ((to - 1) / m | 0) * m;
  if (lower > upper) return 0;
  return (upper - lower) / m + 1;
}

let mySolution = (n, m) => {
    const length = String(n).length;
    const dp = new Array(length).fill(0);

    const digit = i => parseInt(String(n)[i]); 
    dp[0] = delsInRange(1, digit(0), m);

    for (let i = 1; i < length; i++) {
        let from = 10**i;
        let to = parseInt(String(n).slice(0, i + 1));
        dp[i] = dp[i - 1] + delsInRange(from, to, m);
        let before = parseInt(String(n).slice(0, i));
        if (before % m == 0) dp[i]++;
    }
  
    return dp[length - 1];
}

for (let n = 1; n < 100; n++) {
  for (let m = 1; m <= n; m++) {
    let iters = f(n, m);
    let my = mySolution(n, m);
    if (iters != my) {
      console.log("failed", "n =", n, "m =", m, "f(n, m) =", iters, "my(n, m) =", my);
    }
  }
}
