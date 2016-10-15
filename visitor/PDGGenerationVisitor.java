package visitor;

import ast.*;
import lombok.AllArgsConstructor;
import lombok.Value;
import pdg.PDGNode;

import java.util.*;
import java.util.stream.Collectors;

@Value
public class PDGGenerationVisitor extends ASTGeneralVisitor {

    private Map<BlockItem, PDGNode> allNodes = new IdentityHashMap<>();

    public void link(BlockItem from, BlockItem to) {
        if (!allNodes.containsKey(from)) {
            allNodes.put(from, new PDGNode(from, new ArrayList<>(), new ArrayList<>()));
        }
        if (!allNodes.containsKey(to)) {
            allNodes.put(to, new PDGNode(to, new ArrayList<>(), new ArrayList<>()));
        }
        PDGNode fromNode = allNodes.get(from), toNode = allNodes.get(to);
        fromNode.getIsADependencyFor().add(toNode);
        //toNode.getDependsOn().add(fromNode);
    }

    public void printAdjacencyMatrix() {
        int i = 0;
        String label;
        System.out.println("Identity Matrix");
        Map<PDGNode, String> labels = new IdentityHashMap<>();
        for (PDGNode pdgNode : allNodes.values()) {
            label = String.valueOf(Character.valueOf((char)('A' + i)));
            labels.put(pdgNode, label);
            System.out.println(label + " := " + pdgNode.getBlockItem().toCode());
            i++;
        }

        for (PDGNode pdgNode : allNodes.values()) {
            String source = labels.get(pdgNode);
            System.out.print(source + ": ");
            List<String> destLabels = pdgNode.getIsADependencyFor().stream().map(node -> labels.get(node)).collect(Collectors.toList());
            System.out.println(String.join(", ", destLabels));
        }
    }


    @Value
    @AllArgsConstructor
    class Dependencies {
        Set<String> dependentVariables;
        Set<String> changedVariables;

        Dependencies() {
            dependentVariables = Collections.emptySet();
            changedVariables = Collections.emptySet();
        }

        Dependencies add(Dependencies d) {
            Set<String> otherDependentVariables = new HashSet<>(d.dependentVariables);
            otherDependentVariables.removeAll(this.changedVariables);
            this.dependentVariables.addAll(otherDependentVariables);
            this.changedVariables.addAll(d.changedVariables);
            return this;
        }
    }

    private void nullableExpression(Expression expression, Dependencies dependencies) {
        if (expression != null) {
            if (expression instanceof Assigning) {
                Set<String> rightVariables = ((Assigning) expression).getRightVariables();
                System.out.println(rightVariables);
                rightVariables.removeAll(dependencies.changedVariables);
                dependencies.dependentVariables.addAll(rightVariables);
                dependencies.changedVariables.addAll(expression.getLValues());
            } else {
                Set<String> variables = expression.getVariables();
                variables.removeAll(dependencies.changedVariables);
                dependencies.dependentVariables.addAll(variables);
            }
        }
    }

    public PDGGenerationVisitor(Program p) {
        super(p);
        visit(p);
        printAdjacencyMatrix();
    }

    private void visit(Program p) {
        for (Function function : p.getFunction()) {
            visit(function.getCompoundStatement());
        }
    }

    private Dependencies visit(Statement statement) {
        if (statement instanceof CompoundStatement) {
            return visit(((CompoundStatement) statement));
        } else if (statement instanceof ExpressionStatement) {
            return visit(((ExpressionStatement) statement));
        } else if (statement instanceof SelectionStatementIf) {
            return visit(((SelectionStatementIf) statement));
        } else if (statement instanceof IterationStatementFor) {
            return visit(((IterationStatementFor) statement));
        } else if (statement instanceof IterationStatementDeclareFor) {
            return visit(((IterationStatementDeclareFor) statement));
        } else if (statement instanceof IterationStatementWhile) {
            return visit(((IterationStatementWhile) statement));
        } else if (statement instanceof IterationStatementDoWhile) {
            return visit(((IterationStatementDoWhile) statement));
        } else {
            return new Dependencies(statement.getDependantVariables(), statement.getChangedVariables());
        }
    }

    private Dependencies visit(IterationStatementFor statement) {
        Dependencies dependencies = new Dependencies();
        nullableExpression(statement.getInitial(), dependencies);
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());
        bodyDependencies.dependentVariables.removeAll(dependencies.changedVariables);
        dependencies.dependentVariables.addAll(bodyDependencies.dependentVariables);
        dependencies.changedVariables.addAll(bodyDependencies.changedVariables);
        nullableExpression(statement.getIteration(), dependencies);
        return dependencies;
    }

    private Dependencies visit(IterationStatementDeclareFor statement) {
        Set<String> declaredVariables = new HashSet<>();
        Dependencies dependencies = new Dependencies();
        for (Declaration.DeclaredVariable declaredVariable : statement.getDeclaration().getDeclaredVariables()) {
            nullableExpression(declaredVariable.getInitializer(), dependencies);
            declaredVariables.add(declaredVariable.getIdentifier());
        }
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());
        bodyDependencies.dependentVariables.removeAll(dependencies.changedVariables);
        dependencies.dependentVariables.addAll(bodyDependencies.dependentVariables);
        bodyDependencies.changedVariables.removeAll(declaredVariables);
        dependencies.changedVariables.addAll(bodyDependencies.changedVariables);
        nullableExpression(statement.getIteration(), dependencies);
        return dependencies;
    }

    private Dependencies visit(IterationStatementWhile statement) {
        Dependencies dependencies = new Dependencies();
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());
        dependencies.add(bodyDependencies);
        return dependencies;
    }

    private Dependencies visit(IterationStatementDoWhile statement) {
        Dependencies dependencies = new Dependencies();
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());
        dependencies.add(bodyDependencies);
        return dependencies;
    }

    private Dependencies visit(ExpressionStatement statement) {
        return visit(statement.getExpression());
    }

    private Dependencies visit(Expression expression) {
        if (expression instanceof Assigning) {
            Assigning assigning = (Assigning) expression;
            Set<String> usedVariables = assigning.getRightVariables();
            Set<String> lValues = assigning.getLValues();
            return new Dependencies(usedVariables, lValues);
        } else {
            return new Dependencies(expression.getVariables(), Collections.emptySet());
        }
    }

    private Dependencies visit(CompoundStatement statement) {
        Map<String, BlockItem> lastAssigned = new HashMap<>();
        Set<String> incomingValues = new HashSet<>();
        for (BlockItem blockItem : statement.getBlockItems()) {
            if (blockItem instanceof Statement) {
                Dependencies dependencies = visit(((Statement) blockItem));
                System.out.println("\nProcessing for " + blockItem);
                System.out.println(dependencies);
                printLastAssigned(lastAssigned);
                for (String usedVariable : dependencies.getDependentVariables()) {
                    if (!lastAssigned.containsKey(usedVariable)) {
                        incomingValues.add(usedVariable);
                    } else {
                        System.out.println("Depends on " + usedVariable);
                        BlockItem from = lastAssigned.get(usedVariable);
                        BlockItem to = blockItem;
                        link(from, to);
                    }
                }
                for (String changedVariables : dependencies.getChangedVariables()) {
                    lastAssigned.put(changedVariables, blockItem);
                }
                System.out.println("---");
                printLastAssigned(lastAssigned);
            } else if (blockItem instanceof Declaration) {
                Declaration declaration = (Declaration) blockItem;
                for (Declaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
                    if (declaredVariable.getInitializer() instanceof Assigning) {
                        lastAssigned.put(declaredVariable.getIdentifier(), declaration);
                        Set<String> rightVariables = ((Assigning) declaredVariable.getInitializer()).getRightVariables();
                        rightVariables.removeAll(lastAssigned.keySet());
                        incomingValues.addAll(rightVariables);
                        for (String lvalue : declaredVariable.getInitializer().getLValues()) {
                            lastAssigned.put(lvalue, declaration);
                        }
                    }
                }
            }
        }
        return new Dependencies(incomingValues, lastAssigned.keySet());
    }

    private Dependencies visit(SelectionStatementIf statement) {
        Set<String> dependentVariables = new HashSet<>();
        Set<String> assignedVariables = new HashSet<>();
        Dependencies conditionDependencies = visit(statement.getCondition());
        dependentVariables.addAll(conditionDependencies.getDependentVariables());
        assignedVariables.addAll(conditionDependencies.getChangedVariables());

        Statement thenStatement = statement.getThenStatement();
        Dependencies thenDependencies = visit(thenStatement);
        thenDependencies.dependentVariables.removeAll(assignedVariables);
        dependentVariables.addAll(thenDependencies.dependentVariables);


        if (statement.getElseStatement() != null) {
            Dependencies elseDependencies;
            elseDependencies = visit(statement.getElseStatement());
            elseDependencies.dependentVariables.removeAll(assignedVariables);
            dependentVariables.addAll(elseDependencies.dependentVariables);
            assignedVariables.addAll(elseDependencies.changedVariables);
        }
        assignedVariables.addAll(thenDependencies.changedVariables);
        return new Dependencies(dependentVariables, assignedVariables);
    }

    private void printLastAssigned(Map<String, BlockItem> map) {
        for (Map.Entry<String, BlockItem> stringBlockItemEntry : map.entrySet()) {
            System.out.println(stringBlockItemEntry.getKey() + ": " + stringBlockItemEntry.getValue().toCode());
        }
    }
}
