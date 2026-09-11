// ============================================================================
// Salesforce Enterprise Order Management & Billing Orchestration Platform
// Real-world Enterprise Design Patterns (FFLIB / Apex Enterprise Architecture)
// ============================================================================

// ----------------------------------------------------------------------------
// LAYER 1: DATA TRANSFER OBJECTS (DTO) & VALUE OBJECTS
// ----------------------------------------------------------------------------

public class OrderItemDTO {
    public String productSku;
    public String productName;
    public Integer quantity;
    public Double unitPrice;
    public Double discountPct;
    public Double lineTotal;

    public OrderItemDTO(String sku, String name, Integer qty, Double price, Double disc) {
        this.productSku = sku;
        this.productName = name;
        this.quantity = qty;
        this.unitPrice = price;
        this.discountPct = disc;
        this.lineTotal = qty * price * (1.0 - (disc / 100.0));
    }
}

public class OrderRequestDTO {
    public String customerCode;
    public String customerTier;
    public List<OrderItemDTO> items;
    public String currencyIsoCode;
    public Boolean requireExpeditedShipping;

    public OrderRequestDTO(String code, String tier, String currCode) {
        this.customerCode = code;
        this.customerTier = tier;
        this.currencyIsoCode = currCode;
        this.requireExpeditedShipping = false;
        this.items = new List<OrderItemDTO>();
    }

    public void addItem(OrderItemDTO item) {
        this.items.add(item);
    }
}

public class OrderCalculationResult {
    public Double subtotal;
    public Double totalDiscount;
    public Double taxAmount;
    public Double shippingFee;
    public Double grandTotal;
    public String taxJurisdiction;
    public Boolean approvedForFulfillment;

    public OrderCalculationResult() {
        this.subtotal = 0.0;
        this.totalDiscount = 0.0;
        this.taxAmount = 0.0;
        this.shippingFee = 0.0;
        this.grandTotal = 0.0;
        this.approvedForFulfillment = false;
    }
}

// ----------------------------------------------------------------------------
// LAYER 2: BUSINESS ENGINE SERVICES (Pricing, Tax, Credit Check)
// ----------------------------------------------------------------------------

public class PricingAndDiscountEngine {
    public static Double calculateVolumeDiscount(Integer totalQuantity, String customerTier) {
        Double discount = 0.0;
        
        // Tiered Volume Strategy
        if (totalQuantity >= 100) {
            discount = 15.0;
        } else if (totalQuantity >= 50) {
            discount = 10.0;
        } else if (totalQuantity >= 20) {
            discount = 5.0;
        }

        // VIP / Platinum Bonus
        if (customerTier == 'PLATINUM' || customerTier == 'ENTERPRISE') {
            discount += 5.0;
        } else if (customerTier == 'GOLD') {
            discount += 2.5;
        }

        return Math.min(25.0, discount);
    }
}

public class TaxCalculationService {
    public static Double calculateTax(Double netAmount, String jurisdiction) {
        Double taxRate = 0.0;
        
        if (jurisdiction == 'EU_DE') {
            taxRate = 0.19;
        } else if (jurisdiction == 'EU_FR') {
            taxRate = 0.20;
        } else if (jurisdiction == 'US_CA') {
            taxRate = 0.0825;
        } else if (jurisdiction == 'US_NY') {
            taxRate = 0.08875;
        } else {
            taxRate = 0.10;
        }

        return netAmount * taxRate;
    }
}

public class AccountCreditService {
    public static Boolean verifyCreditLimit(Account customer, Double requestedAmount) {
        if (customer == null) {
            return false;
        }
        Double creditLimit = customer.AnnualRevenue * 0.15;
        System.debug('[CREDIT_SERVICE] Evaluating Credit Limit for: ' + customer.Name);
        System.debug('[CREDIT_SERVICE] Available Credit: $' + creditLimit + ' | Requested Order: $' + requestedAmount);
        
        return requestedAmount <= creditLimit;
    }
}

// ----------------------------------------------------------------------------
// LAYER 3: DOMAIN & TRIGGER HANDLERS
// ----------------------------------------------------------------------------

public class OrderTriggerHandler {
    public static void beforeInsert(List<Order> newOrders) {
        System.debug('[TRIGGER_HANDLER] Running OrderTriggerHandler.beforeInsert for ' + newOrders.size() + ' orders');
        for (Order ord : newOrders) {
            if (String.isBlank(ord.Status)) {
                ord.Status = 'Draft';
            }
            ord.CreatedDate = System.today();
        }
    }

    public static void afterInsert(List<Order> newOrders) {
        System.debug('[TRIGGER_HANDLER] Running OrderTriggerHandler.afterInsert - Dispatching Async Events');
        for (Order ord : newOrders) {
            System.debug('[EVENT_BUS] Published OrderCreatedEvent for OrderId=' + ord.Id + ' Status=' + ord.Status);
        }
    }
}

// ----------------------------------------------------------------------------
// LAYER 4: ORDER ORCHESTRATION & TRANSACTION MANAGEMENT
// ----------------------------------------------------------------------------

public class OrderOrchestrationService {
    public static OrderCalculationResult processOrder(OrderRequestDTO req, Account customer, String jurisdiction) {
        System.debug('===============================================================');
        System.debug('>>> [ORCHESTRATOR] Starting Order Workflow for: ' + req.customerCode + ' <<<');
        System.debug('===============================================================');

        OrderCalculationResult result = new OrderCalculationResult();
        result.taxJurisdiction = jurisdiction;

        // Step 1: Calculate raw line items and quantities
        Integer totalUnits = 0;
        Double rawSubtotal = 0.0;
        
        for (OrderItemDTO item : req.items) {
            totalUnits += item.quantity;
            rawSubtotal += (item.quantity * item.unitPrice);
            System.debug(' - SKU: ' + item.productSku + ' (' + item.productName + ') x' + item.quantity + ' @ $' + item.unitPrice + ' -> Line: $' + item.lineTotal);
        }
        result.subtotal = rawSubtotal;

        // Step 2: Calculate Global Tier & Volume Discount
        Double tierDiscountPct = PricingAndDiscountEngine.calculateVolumeDiscount(totalUnits, req.customerTier);
        Double discountAmount = rawSubtotal * (tierDiscountPct / 100.0);
        Double netAmount = rawSubtotal - discountAmount;
        result.totalDiscount = discountAmount;

        System.debug('[PRICING] Total Units: ' + totalUnits + ' | Applied Discount: ' + tierDiscountPct + '% ($' + discountAmount + ')');

        // Step 3: Calculate Multi-Region Tax
        Double tax = TaxCalculationService.calculateTax(netAmount, jurisdiction);
        result.taxAmount = tax;
        System.debug('[TAX] Jurisdiction: ' + jurisdiction + ' | Calculated Tax: $' + tax);

        // Step 4: Shipping logic
        Double shipping = 25.0;
        if (req.requireExpeditedShipping) {
            shipping = 75.0;
        }
        if (netAmount > 1000.0) {
            shipping = 0.0; // Free shipping threshold
        }
        result.shippingFee = shipping;

        // Step 5: Final Grand Total
        result.grandTotal = netAmount + tax + shipping;
        System.debug('[TOTAL] Net: $' + netAmount + ' + Tax: $' + tax + ' + Shipping: $' + shipping + ' ==> Grand Total: $' + result.grandTotal);

        // Step 6: Transaction Boundary & Credit Check
        Savepoint sp = Database.setSavepoint();
        try {
            Boolean creditApproved = AccountCreditService.verifyCreditLimit(customer, result.grandTotal);
            if (!creditApproved) {
                System.debug('[SECURITY_ALERT] Order rejected: Credit limit exceeded for customer ' + customer.Name);
                result.approvedForFulfillment = false;
                return result;
            }

            // Step 7: Persist Order SObject
            Order ord = new Order(
                AccountId = customer.Id,
                Status = 'Activated',
                TotalAmount = result.grandTotal,
                NetPrice = netAmount,
                TaxAmount = result.taxAmount,
                OrderNumber = 'ORD-2026-9901'
            );

            // Simulate Trigger lifecycle
            List<Order> orderList = new List<Order>{ ord };
            OrderTriggerHandler.beforeInsert(orderList);
            insert ord;
            OrderTriggerHandler.afterInsert(orderList);

            System.debug('[DATABASE] Successfully persisted Order record: ' + ord.Id + ' [OrderNumber=' + ord.OrderNumber + ']');
            result.approvedForFulfillment = true;

        } catch (Exception ex) {
            System.debug('[TRANSACTION_ERROR] Exception caught during order orchestration. Rolling back...');
            Database.rollback(sp);
            result.approvedForFulfillment = false;
        }

        return result;
    }
}

// ----------------------------------------------------------------------------
// LAYER 5: BATCH PROCESSING & INVOICING ENGINE
// ----------------------------------------------------------------------------

public class BatchInvoiceGenerator {
    public static Integer executeBatchRun(List<Order> ordersToProcess) {
        System.debug('---------------------------------------------------------------');
        System.debug('>>> [BATCH_JOB] Executing BatchInvoiceGenerator (Batch Size=' + ordersToProcess.size() + ') <<<');
        System.debug('---------------------------------------------------------------');

        Integer invoiceCount = 0;
        for (Order o : ordersToProcess) {
            String invNumber = 'INV-' + o.Id;
            Double invoiceAmount = o.TotalAmount;
            
            System.debug('[INVOICE_GENERATED] ' + invNumber + ' for AccountId: ' + o.AccountId + ' Amount: $' + invoiceAmount + ' Status: Issued');
            invoiceCount++;
        }

        return invoiceCount;
    }
}

// test scenario execution
System.debug('running order test');

// 1. create test account and contact
Account corpAccount = new Account(
    Name = 'Acme Corp',
    Industry = 'Tech',
    AnnualRevenue = 250000.0,
    Rating = 'Hot',
    CustomerTier = 'PLATINUM'
);
insert corpAccount;
System.debug('created account: ' + corpAccount.Name + ' id=' + corpAccount.Id);

Contact billingContact = new Contact(
    AccountId = corpAccount.Id,
    FirstName = 'Eleanor',
    LastName = 'Vance',
    Email = 'e.vance@acme.test',
    Title = 'Buyer'
);
insert billingContact;
System.debug('created contact: ' + billingContact.FirstName + ' ' + billingContact.LastName);

// 2. build order items
OrderRequestDTO orderReq = new OrderRequestDTO('ACME-01', corpAccount.CustomerTier, 'USD');
orderReq.addItem(new OrderItemDTO('SKU-SRV-900', 'Server Rack', 12, 1200.0, 0.0));
orderReq.addItem(new OrderItemDTO('SKU-NET-400', 'Fiber Switch', 8, 450.0, 5.0));
orderReq.addItem(new OrderItemDTO('SKU-LIC-880', 'Cloud License', 50, 150.0, 10.0));

// 3. process order
OrderCalculationResult calcResult = OrderOrchestrationService.processOrder(orderReq, corpAccount, 'US_CA');

// 4. query db
List<Order> activeOrders = [SELECT Id, AccountId, Status, TotalAmount, OrderNumber FROM Order WHERE AccountId = :corpAccount.Id];
System.debug('orders in db: ' + activeOrders.size());

// 5. generate invoice
Integer generatedInvoices = BatchInvoiceGenerator.executeBatchRun(activeOrders);

// 6. stats
System.debug('soql queries: ' + Limits.getQueries() + ' / ' + Limits.getLimitQueries());
System.debug('dml stmts: ' + Limits.getDmlStatements() + ' / ' + Limits.getLimitDmlStatements());
System.debug('user: ' + UserInfo.getUserName());
System.debug('test completed successfully');
