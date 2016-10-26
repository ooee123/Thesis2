package ast;

import java.util.Comparator;

/**
 * Created by ooee on 10/25/16.
 */
public class BlockItemComparator<T extends BlockItem> implements Comparator<T> {

    public enum BlockItemStrictOrdering {
        DECLARATION(Declaration.class),
        EXPRESSION_STATEMENT(ExpressionStatement.class),
        SELECTION_STATEMENT_IF(SelectionStatementIf.class),
        SELECTION_STATEMENT_SWITCH(SelectionStatementSwitch.class),
        ITERATION_STATEMENT_FOR(IterationStatementFor.class),
        ITERATION_STATEMENT_DECLARE_FOR(IterationStatementDeclareFor.class),
        ITERATION_STATEMENT_WHILE(IterationStatementWhile.class),
        ITERATION_STATEMENT_DO_WHILE(IterationStatementDoWhile.class),
        LABELED_IDENTIFIER_STATEMENT(LabeledIdentifierStatement.class),
        LABELED_CASE_STATEMENT(LabeledCaseStatement.class),
        LABELED_DEFAULT_STATEMENT(LabeledDefaultStatement.class),
        COMPOUND_STATEMENT(CompoundStatement.class);

        private Class<? extends BlockItem> clazz;
        BlockItemStrictOrdering(Class<? extends BlockItem> clazz) {
            this.clazz = clazz;
        }

        public static int getIndex(BlockItem blockItem) {
            Class<? extends BlockItem> blockItemClass = blockItem.getClass();
            BlockItemStrictOrdering[] values = BlockItemStrictOrdering.values();
            for (int i = 0; i < values.length; i++) {
                if (blockItemClass.equals(values[i])) {
                    return i;
                }
            }
            throw new IllegalArgumentException("Block Item is not strictly ordered. Received " + blockItem.getClass());
        }
    }

    @Override
    public int compare(BlockItem b1, BlockItem b2) {
        if (b1.getClass() != b2.getClass()) {
            return Integer.compare(BlockItemStrictOrdering.getIndex(b1), BlockItemStrictOrdering.getIndex(b2));
        } else {
            return Integer.compare(b1.pointValue(), b2.pointValue());
        }
    }
}
