typedef struct {
    char *name;
    int age;
} Person;

typedef struct {
    Person person;
    int id;
} Student;

int main() {
    int a, b, c;
    Student student;
    student.person.age = 17;
    student.person.name = "Kevin";

    if (student.person.age > 18) {
        student.person.name = "REDACTED";
    }
    return student.person.name;
}