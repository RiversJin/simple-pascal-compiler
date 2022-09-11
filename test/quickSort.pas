program prog;
var nums:array[1..10] of integer;i:integer;x:integer;
function partion(low:integer;high:integer):integer;
    var pivot:integer;i:integer;j:integer;temp:integer;
    begin
        pivot:=nums[high];
        i:=low-1;
        j:=low;
        repeat
            if nums[j] < pivot then begin
                i:=i+1;
                temp:=nums[i];
                nums[i]:=nums[j];
                nums[j]:=temp;
            end;
            j:=j+1;
        until j >= high;
        temp:=nums[i+1];
        nums[i+1]:=nums[high];
        nums[high]:=temp;
        partion:=i+1;
    end;
procedure quickSort(low:integer;high:integer);
    var mid:integer;
    begin
        if low < high then begin
            mid:=partion(low,high);
            quickSort(low,mid-1);
            quickSort(mid+1,high);
        end;
    end;
begin
    for i:=1 to 10 do begin
        readln(x);
        nums[i]:=x;
        
    end;
    writeln('Echo');
    for i:=1 to 10 do begin
        writeln(nums[i]);
    end;
    writeln('Start to sort');
    quickSort(1,10);
    for i:=1 to 10 do begin
        writeln(nums[i]);
    end;
end.
