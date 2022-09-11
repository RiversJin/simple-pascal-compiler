{这个文件用于测试结构体}
program exRecords;
type
Books = record
   title: integer;
end;
var
   Book1, Book2: Books; (* Declare Book1 and Book2 of type Books *)
begin
   Book1.title  := 1;
end.