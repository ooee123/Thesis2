package ast;

/**
 * Created by ooee on 9/26/16.
 */
public class AssignmentExpressionImpl implements AssignmentExpression {
    public enum AssignmentOperator {
        ASSIGNMENT,
        MULT_ASSIGNMENT,
        DIV_ASSIGNMENT,
        MOD_ASSIGNMENT,
        ADD_ASSIGNMENT,
        SUB_ASSIGNMENT,
        LEFT_SHIFT_ASSIGNMENT,
        RIGHT_SHIFT_ASSIGNMENT,
        BIT_AND_ASSIGNMENT,
        BIT_XOR_ASSIGNMENT,
        BIT_OR_ASSIGNMENT;
    }

    UnaryExpression unaryExpression;
    AssignmentOperator assignmentOperator;
    AssignmentExpressionImpl assignmentExpression;
}
