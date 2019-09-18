a,b=map(int,input().split())
arr=[]
for i in range(0, a):
    arr.append(list(map(int, input().split())))
pathcount = 0

def PathSearch(row, col):
    global pathcount
    if row == a-1 and col == b-1:
        pathcount +=1
        return
    if row -1 >= 0:
        if arr[row-1][col] < arr[row][col]:
            PathSearch(row-1, col)
    if col -1 >= 0:
        if arr[row][col-1] < arr[row][col]:
            PathSearch(row, col-1)
    if row +1<a:
        if arr[row+1][col] < arr[row][col]:
            PathSearch(row+1, col)
    if col +1<b:
        if arr[row][col+1] < arr[row][col]:
            PathSearch(row, col+1)

PathSearch(0,0)
print(pathcount)