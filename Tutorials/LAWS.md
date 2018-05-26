`i in a ->> i` == `a`

`a = a` == `a`

`a _= 1` == `a = a _ 1`

`a ._ b` == `[ a[0] _ b[0], a[1] _ b[1], ... a[n - 1] _ b[n - 1], a[n] _ b[n] ]`

`a ->> b` == `[ b, ... (-> len a) b ]`

`a push b` == `a ++= [ b ]`

`a post b` == `a = [ b ] ++ a`

`pop a` == `a = a[0 : end - 1]`

`mov a` == `a = a[1 : end]`

`[a : b]` == `[a :> b - 1]`